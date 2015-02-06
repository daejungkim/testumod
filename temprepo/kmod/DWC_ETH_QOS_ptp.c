/**********************************************************************
 *
 * Copyright (c) Vayavya Labs Pvt. Ltd. 2008
 *
 * THIS IS A DEVICE DRIVER GENERATED USING VAYAVYA LABS' DDGen TOOL.
 * THIS DRIVER WAS GENERATED FOR THE FOLLOWING SPECS:
 *
 * Device: DWC_ETH_QOS
 * Device Manufacturer: SYNOPSYS
 * Operating System: Linux
 *
 * DPS Reference ID: 5811729:880:146:648
 * RTS Reference ID: 63217:75:131078:0
 *
 * THE SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND. TO
 * THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, VAYAVYA LABS Pvt Ltd.
 * DISCLAIMS ALL WARRANTIES, EXPRESSED OR IMPLIED, INCLUDING, BUT NOT
 * LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE AND ANY WARRANTY AGAINST INFRINGEMENT, WITH REGARD
 * TO THE SOFTWARE.
 *
 * VAYAVYA LABS Pvt.Ltd. SHALL BE RELIEVED OF ANY AND ALL OBLIGATIONS
 * WITH RESPECT TO THIS SECTION FOR ANY PORTIONS OF THE SOFTWARE THAT
 * ARE REVISED, CHANGED, MODIFIED, OR MAINTAINED BY ANYONE OTHER THAN
 * VAYAVYA LABS Pvt.Ltd.
 *
 ***********************************************************************/

/*!@file: DWC_ETH_QOS_ptp.c
 * @brief: Driver functions.
 */
#include "DWC_ETH_QOS_yheader.h"


/*!
 * \brief API to adjust the frequency of hardware clock.
 *
 * \details This function is used to adjust the frequency of the
 * hardware clock.
 *
 * \param[in] ptp – pointer to ptp_clock_info structure.
 * \param[in] delta – desired period change in parts per billion.
 *
 * \return int
 *
 * \retval 0 on success and -ve number on failure.
 */

static int DWC_ETH_QOS_adjust_freq(struct ptp_clock_info *ptp, s32 ppb)
{
	struct DWC_ETH_QOS_prv_data *pdata =
		container_of(ptp, struct DWC_ETH_QOS_prv_data, ptp_clock_ops);
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	unsigned long flags;
	u64 adj;
	u32 diff, addend;
	int neg_adj = 0;

	DBGPR_PTP("-->DWC_ETH_QOS_adjust_freq: %d\n", ppb);

	if (ppb < 0) {
		neg_adj = 1;
		ppb = -ppb;
	}

	addend = pdata->default_addend;
	adj = addend;
	adj *= ppb;
	/*
	 * div_u64 will divided the "adj" by "1000000000ULL"
	 * and return the quotient.
	 * */
	diff = div_u64(adj, 1000000000ULL);
	addend = neg_adj ? (addend - diff) : (addend + diff);

	spin_lock_irqsave(&pdata->ptp_lock, flags);

	hw_if->config_addend(addend);

	spin_unlock_irqrestore(&pdata->ptp_lock, flags);

	DBGPR_PTP("<--DWC_ETH_QOS_adjust_freq\n");

	return 0;
}


/*!
 * \brief API to adjust the hardware time.
 *
 * \details This function is used to shift/adjust the time of the
 * hardware clock.
 *
 * \param[in] ptp – pointer to ptp_clock_info structure.
 * \param[in] delta – desired change in nanoseconds.
 *
 * \return int
 *
 * \retval 0 on success and -ve number on failure.
 */

static int DWC_ETH_QOS_adjust_time(struct ptp_clock_info *ptp, s64 delta)
{
	struct DWC_ETH_QOS_prv_data *pdata =
		container_of(ptp, struct DWC_ETH_QOS_prv_data, ptp_clock_ops);
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	unsigned long flags;
	u32 sec, nsec;
	u32 quotient, reminder;
	int neg_adj = 0;

	DBGPR_PTP("-->DWC_ETH_QOS_adjust_time: delta = %lld\n", delta);

	if (delta < 0) {
		neg_adj = 1;
		delta =-delta;
	}

	quotient = div_u64_rem(delta, 1000000000ULL, &reminder);
	sec = quotient;
	nsec = reminder;

	spin_lock_irqsave(&pdata->ptp_lock, flags);

	hw_if->adjust_systime(sec, nsec, neg_adj, pdata->one_nsec_accuracy);

	spin_unlock_irqrestore(&pdata->ptp_lock, flags);

	DBGPR_PTP("<--DWC_ETH_QOS_adjust_time\n");

	return 0;
}


/*!
 * \brief API to get the current time.
 *
 * \details This function is used to read the current time from the
 * hardware clock.
 *
 * \param[in] ptp – pointer to ptp_clock_info structure.
 * \param[in] ts – pointer to hold the time/result.
 *
 * \return int
 *
 * \retval 0 on success and -ve number on failure.
 */

static int DWC_ETH_QOS_get_time(struct ptp_clock_info *ptp,
	struct timespec *ts)
{
	struct DWC_ETH_QOS_prv_data *pdata =
		container_of(ptp, struct DWC_ETH_QOS_prv_data, ptp_clock_ops);
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	u64 ns;
	u32 reminder;
	unsigned long flags;

	DBGPR_PTP("-->DWC_ETH_QOS_get_time\n");

	spin_lock_irqsave(&pdata->ptp_lock, flags);

	ns = hw_if->get_systime();

	spin_unlock_irqrestore(&pdata->ptp_lock, flags);

	ts->tv_sec = div_u64_rem(ns, 1000000000ULL, &reminder);
	ts->tv_nsec = reminder;

	DBGPR_PTP("<--DWC_ETH_QOS_get_time: ts->tv_sec = %ld,"
		"ts->tv_nsec = %ld\n", ts->tv_sec, ts->tv_nsec);

	return 0;
}


/*!
 * \brief API to set the current time.
 *
 * \details This function is used to set the current time on the
 * hardware clock.
 *
 * \param[in] ptp – pointer to ptp_clock_info structure.
 * \param[in] ts – time value to set.
 *
 * \return int
 *
 * \retval 0 on success and -ve number on failure.
 */

static int DWC_ETH_QOS_set_time(struct ptp_clock_info *ptp,
	const struct timespec *ts)
{
	struct DWC_ETH_QOS_prv_data *pdata =
		container_of(ptp, struct DWC_ETH_QOS_prv_data, ptp_clock_ops);
	struct hw_if_struct *hw_if = &(pdata->hw_if);
	unsigned long flags;

	DBGPR_PTP("-->DWC_ETH_QOS_set_time: ts->tv_sec = %ld,"
		"ts->tv_nsec = %ld\n", ts->tv_sec, ts->tv_nsec);

	spin_lock_irqsave(&pdata->ptp_lock, flags);

	hw_if->init_systime(ts->tv_sec, ts->tv_nsec);

	spin_unlock_irqrestore(&pdata->ptp_lock, flags);

	DBGPR_PTP("<--DWC_ETH_QOS_set_time\n");

	return 0;
}


/*!
 * \brief API to enable/disable an ancillary feature.
 *
 * \details This function is used to enable or disable an ancillary
 * device feature like PPS, PEROUT and EXTTS.
 *
 * \param[in] ptp – pointer to ptp_clock_info structure.
 * \param[in] rq – desired resource to enable or disable.
 * \param[in] on – caller passes one to enable or zero to disable.
 *
 * \return int
 *
 * \retval 0 on success and -ve(EINVAL or EOPNOTSUPP) number on failure.
 */

static int DWC_ETH_QOS_enable(struct ptp_clock_info *ptp,
	struct ptp_clock_request *rq, int on)
{
	return -EOPNOTSUPP;
}

/*
 * structure describing a PTP hardware clock.
 */
static struct ptp_clock_info DWC_ETH_QOS_ptp_clock_ops = {
	.owner = THIS_MODULE,
	.name = "DWC_ETH_QOS_clk",
	.max_adj = DWC_ETH_QOS_SYSCLOCK, /* the max possible frequency adjustment,
				in parts per billion */
	.n_alarm = 0,	/* the number of programmable alarms */
	.n_ext_ts = 0,	/* the number of externel time stamp channels */
	.n_per_out = 0, /* the number of programmable periodic signals */
	.pps = 0,	/* indicates whether the clk supports a PPS callback */
	.adjfreq = DWC_ETH_QOS_adjust_freq,
	.adjtime = DWC_ETH_QOS_adjust_time,
	.gettime = DWC_ETH_QOS_get_time,
	.settime = DWC_ETH_QOS_set_time,
	.enable = DWC_ETH_QOS_enable,
};


/*!
 * \brief API to register ptp clock driver.
 *
 * \details This function is used to register the ptp clock
 * driver to kernel. It also does some housekeeping work.
 *
 * \param[in] pdata – pointer to private data structure.
 *
 * \return int
 *
 * \retval 0 on success and -ve number on failure.
 */

int DWC_ETH_QOS_ptp_init(struct DWC_ETH_QOS_prv_data *pdata)
{
	int ret = 0;

	DBGPR_PTP("-->DWC_ETH_QOS_ptp_init\n");

	if (!pdata->hw_feat.tsstssel) {
		ret = -1;
		pdata->ptp_clock = NULL;
		printk(KERN_ALERT "No PTP supports in HW\n"
			"Aborting PTP clock driver registration\n");
		goto no_hw_ptp;
	}

	spin_lock_init(&pdata->ptp_lock);

	pdata->ptp_clock_ops = DWC_ETH_QOS_ptp_clock_ops;

	pdata->ptp_clock = ptp_clock_register(&pdata->ptp_clock_ops, &(pdata->pdev->dev));
	if (IS_ERR(pdata->ptp_clock)) {
		pdata->ptp_clock = NULL;
		printk(KERN_ALERT "ptp_clock_register() failed\n");
	} else
		printk(KERN_ALERT "Added PTP HW clock successfully\n");

	DBGPR_PTP("<--DWC_ETH_QOS_ptp_init\n");

	return ret;

no_hw_ptp:
	return ret;
}


/*!
 * \brief API to unregister ptp clock driver.
 *
 * \details This function is used to remove/unregister the ptp
 * clock driver from the kernel.
 *
 * \param[in] pdata – pointer to private data structure.
 *
 * \return void
 */

void DWC_ETH_QOS_ptp_remove(struct DWC_ETH_QOS_prv_data *pdata)
{
	DBGPR_PTP("-->DWC_ETH_QOS_ptp_remove\n");

	if (pdata->ptp_clock) {
		ptp_clock_unregister(pdata->ptp_clock);
		printk(KERN_ALERT "Removed PTP HW clock successfully\n");
	}

	DBGPR_PTP("<--DWC_ETH_QOS_ptp_remove\n");
}

