/*
 * Copyright (c) 2012 Jan Vesely
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - The name of the author may not be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @defgroup root_amdm37x TI AM/DM37x platform driver.
 * @brief HelenOS TI AM/DM37x platform driver.
 * @{
 */

/** @file
 */

#include <ddf/driver.h>
#include <ddf/log.h>
#include <errno.h>
#include <ops/hw_res.h>
#include <stdio.h>
#include <ddi.h>

#include "uhh.h"
#include "usbtll.h"
#include "core_cm.h"
#include "usbhost_cm.h"

#define NAME  "rootamdm37x"

/** Obtain function soft-state from DDF function node */
#define ROOTARM_FUN(fnode) \
	((rootamdm37x_fun_t *) (fnode)->driver_data)

typedef struct {
	hw_resource_list_t hw_resources;
} rootamdm37x_fun_t;

#define OHCI_BASE_ADDRESS  0x48064400
#define OHCI_SIZE  1024
#define EHCI_BASE_ADDRESS  0x48064800
#define EHCI_SIZE  1024

static hw_resource_t ohci_res[] = {
	{
		.type = MEM_RANGE,
		/* See amdm37x TRM page. 3316 for these values */
		.res.io_range = {
			.address = OHCI_BASE_ADDRESS,
			.size = OHCI_SIZE,
			.endianness = LITTLE_ENDIAN
		},
	},
	{
		.type = INTERRUPT,
		.res.interrupt = { .irq = 76 },
	},
};

static const rootamdm37x_fun_t ohci = {
	.hw_resources = {
	    .resources = ohci_res,
	    .count = sizeof(ohci_res)/sizeof(ohci_res[0]),
	}
};

static hw_resource_t ehci_res[] = {
	{
		.type = MEM_RANGE,
		/* See amdm37x TRM page. 3316 for these values */
		.res.io_range = {
			.address = EHCI_BASE_ADDRESS,
			.size = EHCI_SIZE,
			.endianness = LITTLE_ENDIAN
		},
	},
	{
		.type = INTERRUPT,
		.res.interrupt = { .irq = 77 },
	},
};

static const rootamdm37x_fun_t ehci = {
	.hw_resources = {
	    .resources = ehci_res,
	    .count = sizeof(ehci_res)/sizeof(ehci_res[0]),
	}
};

static hw_resource_list_t *rootamdm37x_get_resources(ddf_fun_t *fnode);
static bool rootamdm37x_enable_interrupt(ddf_fun_t *fun);

static hw_res_ops_t fun_hw_res_ops = {
	.get_resource_list = &rootamdm37x_get_resources,
	.enable_interrupt = &rootamdm37x_enable_interrupt,
};

static ddf_dev_ops_t rootamdm37x_fun_ops =
{
	.interfaces[HW_RES_DEV_IFACE] = &fun_hw_res_ops
};

static int usb_clocks(bool on)
{
	usbhost_cm_regs_t *usb_host_cm = NULL;
	core_cm_regs_t *l4_core_cm = NULL;

	int ret = pio_enable((void*)USBHOST_CM_BASE_ADDRESS, USBHOST_CM_SIZE,
	    (void**)&usb_host_cm);
	if (ret != EOK)
		return ret;

	ret = pio_enable((void*)CORE_CM_BASE_ADDRESS, CORE_CM_SIZE,
	    (void**)&l4_core_cm);
	if (ret != EOK)
		return ret;

	assert(l4_core_cm);
	assert(usb_host_cm);
	if (on) {
		l4_core_cm->iclken3 |= CORE_CM_ICLKEN3_EN_USBTLL_FLAG;
		l4_core_cm->fclken3 |= CORE_CM_FCLKEN3_EN_USBTLL_FLAG;
		usb_host_cm->iclken |= USBHOST_CM_ICLKEN_EN_USBHOST;
		usb_host_cm->fclken |= USBHOST_CM_FCLKEN_EN_USBHOST1_FLAG;
		usb_host_cm->fclken |= USBHOST_CM_FCLKEN_EN_USBHOST2_FLAG;
	} else {
		usb_host_cm->fclken &= ~USBHOST_CM_FCLKEN_EN_USBHOST2_FLAG;
		usb_host_cm->fclken &= ~USBHOST_CM_FCLKEN_EN_USBHOST1_FLAG;
		usb_host_cm->iclken &= ~USBHOST_CM_ICLKEN_EN_USBHOST;
		l4_core_cm->fclken3 &= ~CORE_CM_FCLKEN3_EN_USBTLL_FLAG;
		l4_core_cm->iclken3 &= ~CORE_CM_ICLKEN3_EN_USBTLL_FLAG;
	}

	//TODO Unmap those registers.

	return ret;
}

static int usb_tll_init()
{
	tll_regs_t *usb_tll = NULL;
	uhh_regs_t *uhh_conf = NULL;

	int ret = pio_enable((void*)AMDM37x_USBTLL_BASE_ADDRESS,
	    AMDM37x_USBTLL_SIZE, (void**)&usb_tll);
	if (ret != EOK)
		return ret;

	ret = pio_enable((void*)AMDM37x_UHH_BASE_ADDRESS,
	    AMDM37x_UHH_SIZE, (void**)&uhh_conf);
	if (ret != EOK)
		return ret;

	/* Reset USB tll */
	usb_tll->sysconfig |= TLL_SYSCONFIG_SOFTRESET_FLAG;
	ddf_msg(LVL_DEBUG2, "Waiting for USB TLL reset");
	while (!(usb_tll->sysstatus & TLL_SYSSTATUS_RESET_DONE_FLAG));
	ddf_msg(LVL_DEBUG, "USB TLL Reset done.");

	return EOK;
}

static bool rootamdm37x_add_fun(ddf_dev_t *dev, const char *name,
    const char *str_match_id, const rootamdm37x_fun_t *fun)
{
	ddf_msg(LVL_DEBUG, "Adding new function '%s'.", name);
	
	ddf_fun_t *fnode = NULL;
	match_id_t *match_id = NULL;
	
	/* Create new device. */
	fnode = ddf_fun_create(dev, fun_inner, name);
	if (fnode == NULL)
		goto failure;
	
	fnode->driver_data = (void*)fun;
	
	/* Initialize match id list */
	match_id = create_match_id();
	if (match_id == NULL)
		goto failure;
	
	match_id->id = str_match_id;
	match_id->score = 100;
	add_match_id(&fnode->match_ids, match_id);
	
	/* Set provided operations to the device. */
	fnode->ops = &rootamdm37x_fun_ops;
	
	/* Register function. */
	if (ddf_fun_bind(fnode) != EOK) {
		ddf_msg(LVL_ERROR, "Failed binding function %s.", name);
		goto failure;
	}
	
	return true;
	
failure:
	if (match_id != NULL)
		match_id->id = NULL;
	
	if (fnode != NULL) {
		fnode->driver_data = NULL;
		ddf_fun_destroy(fnode);
	}
	
	ddf_msg(LVL_ERROR, "Failed adding function '%s'.", name);
	
	return false;
}

/** Add the root device.
 *
 * @param dev Device which is root of the whole device tree
 *            (both of HW and pseudo devices).
 *
 * @return Zero on success, negative error number otherwise.
 *
 */
static int rootamdm37x_dev_add(ddf_dev_t *dev)
{
	int ret = usb_clocks(true);
	if (ret != EOK) {
		ddf_msg(LVL_FATAL, "Failed to enable USB HC clocks!.\n");
		return ret;
	}

	ret = usb_tll_init();
	if (ret != EOK) {
		ddf_msg(LVL_FATAL, "Failed to init USB TLL!.\n");
		usb_clocks(false);
		return ret;
	}

	/* Register functions */
	if (!rootamdm37x_add_fun(dev, "ohci", "usb/host=ohci", &ohci))
		ddf_msg(LVL_ERROR, "Failed to add OHCI function for "
		    "BeagleBoard-xM platform.");
	if (!rootamdm37x_add_fun(dev, "ehci", "usb/host=ehci", &ehci))
		ddf_msg(LVL_ERROR, "Failed to add EHCI function for "
		    "BeagleBoard-xM platform.");

	return EOK;
}

/** The root device driver's standard operations. */
static driver_ops_t rootamdm37x_ops = {
	.dev_add = &rootamdm37x_dev_add
};

/** The root device driver structure. */
static driver_t rootamdm37x_driver = {
	.name = NAME,
	.driver_ops = &rootamdm37x_ops
};

static hw_resource_list_t *rootamdm37x_get_resources(ddf_fun_t *fnode)
{
	rootamdm37x_fun_t *fun = ROOTARM_FUN(fnode);
	assert(fun != NULL);
	return &fun->hw_resources;
}

static bool rootamdm37x_enable_interrupt(ddf_fun_t *fun)
{
	/* TODO */
	return false;
}

int main(int argc, char *argv[])
{
	printf("%s: HelenOS AM/DM37x(OMAP37x) platform driver\n", NAME);
	ddf_log_init(NAME, LVL_ERROR);
	return ddf_driver_main(&rootamdm37x_driver);
}

/**
 * @}
 */
