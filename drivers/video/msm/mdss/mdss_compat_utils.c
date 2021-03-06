/*
 * Copyright (c) 2013-2014, The Linux Foundation. All rights reserved.
 * Copyright (C) 1994 Martin Schaller
 *
 * 2001 - Documented with DocBook
 * - Brad Douglas <brad@neruo.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/compat.h>
#include <linux/fb.h>

#include <linux/uaccess.h>

#include "mdss_fb.h"
#include "mdss_compat_utils.h"

#define MSMFB_SET_LUT32 _IOW(MSMFB_IOCTL_MAGIC, 131, struct fb_cmap32)
#define MSMFB_HISTOGRAM32 _IOWR(MSMFB_IOCTL_MAGIC, 132,\
					struct mdp_histogram_data32)
#define MSMFB_GET_CCS_MATRIX32  _IOWR(MSMFB_IOCTL_MAGIC, 133, struct mdp_ccs32)
#define MSMFB_SET_CCS_MATRIX32  _IOW(MSMFB_IOCTL_MAGIC, 134, struct mdp_ccs32)
#define MSMFB_OVERLAY_SET32       _IOWR(MSMFB_IOCTL_MAGIC, 135,\
					struct mdp_overlay32)

#define MSMFB_OVERLAY_GET32      _IOR(MSMFB_IOCTL_MAGIC, 140,\
					struct mdp_overlay32)
#define MSMFB_OVERLAY_BLT32       _IOWR(MSMFB_IOCTL_MAGIC, 142,\
					struct msmfb_overlay_blt32)
#define MSMFB_HISTOGRAM_START32	_IOR(MSMFB_IOCTL_MAGIC, 144,\
					struct mdp_histogram_start_req32)

#define MSMFB_OVERLAY_3D32       _IOWR(MSMFB_IOCTL_MAGIC, 147,\
					struct msmfb_overlay_3d32)

#define MSMFB_MIXER_INFO32       _IOWR(MSMFB_IOCTL_MAGIC, 148,\
						struct msmfb_mixer_info_req32)
#define MSMFB_MDP_PP32 _IOWR(MSMFB_IOCTL_MAGIC, 156, struct msmfb_mdp_pp32)
#define MSMFB_BUFFER_SYNC32  _IOW(MSMFB_IOCTL_MAGIC, 162, struct mdp_buf_sync32)
#define MSMFB_OVERLAY_PREPARE32		_IOWR(MSMFB_IOCTL_MAGIC, 169, \
						struct mdp_overlay_list32)

static unsigned int __do_compat_ioctl_nr(unsigned int cmd32)
{
	unsigned int cmd;

	switch (cmd32) {
	case MSMFB_SET_LUT32:
		cmd = MSMFB_SET_LUT;
		break;
	case MSMFB_HISTOGRAM32:
		cmd = MSMFB_HISTOGRAM;
		break;
	case MSMFB_GET_CCS_MATRIX32:
		cmd = MSMFB_GET_CCS_MATRIX;
		break;
	case MSMFB_SET_CCS_MATRIX32:
		cmd = MSMFB_SET_CCS_MATRIX;
		break;
	case MSMFB_OVERLAY_SET32:
		cmd = MSMFB_OVERLAY_SET;
		break;
	case MSMFB_OVERLAY_GET32:
		cmd = MSMFB_OVERLAY_GET;
		break;
	case MSMFB_OVERLAY_BLT32:
		cmd = MSMFB_OVERLAY_BLT;
		break;
	case MSMFB_OVERLAY_3D32:
		cmd = MSMFB_OVERLAY_3D;
		break;
	case MSMFB_MIXER_INFO32:
		cmd = MSMFB_MIXER_INFO;
		break;
	case MSMFB_MDP_PP32:
		cmd = MSMFB_MDP_PP;
		break;
	case MSMFB_BUFFER_SYNC32:
		cmd = MSMFB_BUFFER_SYNC;
		break;
	case MSMFB_OVERLAY_PREPARE32:
		cmd = MSMFB_OVERLAY_PREPARE;
		break;
	default:
		cmd = cmd32;
		break;
	}

	return cmd;
}

static int mdss_fb_compat_buf_sync(struct fb_info *info, unsigned int cmd,
			 unsigned long arg)
{
	struct mdp_buf_sync32 __user *buf_sync32;
	struct mdp_buf_sync __user *buf_sync;
	u32 data;
	int ret;

	buf_sync = compat_alloc_user_space(sizeof(*buf_sync));
	buf_sync32 = compat_ptr(arg);

	if (copy_in_user(&buf_sync->flags, &buf_sync32->flags,
			 3 * sizeof(u32)))
		return -EFAULT;

	if (get_user(data, &buf_sync32->acq_fen_fd) ||
	    put_user(compat_ptr(data), &buf_sync->acq_fen_fd) ||
	    get_user(data, &buf_sync32->rel_fen_fd) ||
	    put_user(compat_ptr(data), &buf_sync->rel_fen_fd) ||
	    get_user(data, &buf_sync32->retire_fen_fd) ||
	    put_user(compat_ptr(data), &buf_sync->retire_fen_fd))
		return -EFAULT;

	ret = mdss_fb_do_ioctl(info, cmd, (unsigned long) buf_sync);
	if (ret) {
		pr_err("%s: failed %d\n", __func__, ret);
		return ret;
	}

	if (copy_in_user(compat_ptr(buf_sync32->rel_fen_fd),
			buf_sync->rel_fen_fd,
			sizeof(int)))
		return -EFAULT;
	if (copy_in_user(compat_ptr(buf_sync32->retire_fen_fd),
			buf_sync->retire_fen_fd,
			sizeof(int)))
		return -EFAULT;

	return ret;
}

static int mdss_fb_compat_set_lut(struct fb_info *info, unsigned long arg)
{
	struct fb_cmap_user __user *cmap;
	struct fb_cmap32 __user *cmap32;
	__u32 data;
	int ret;

	cmap = compat_alloc_user_space(sizeof(*cmap));
	cmap32 = compat_ptr(arg);

	if (copy_in_user(&cmap->start, &cmap32->start, 2 * sizeof(__u32)))
		return -EFAULT;

	if (get_user(data, &cmap32->red) ||
	    put_user(compat_ptr(data), &cmap->red) ||
	    get_user(data, &cmap32->green) ||
	    put_user(compat_ptr(data), &cmap->green) ||
	    get_user(data, &cmap32->blue) ||
	    put_user(compat_ptr(data), &cmap->blue) ||
	    get_user(data, &cmap32->transp) ||
	    put_user(compat_ptr(data), &cmap->transp))
		return -EFAULT;

	ret = mdss_fb_do_ioctl(info, MSMFB_SET_LUT, (unsigned long) cmap);
	if (!ret)
		pr_debug("%s: compat ioctl successful\n", __func__);

	return ret;
}

static int __to_user_pp_params(struct mdp_overlay_pp_params32 *ppp32,
				   struct mdp_overlay_pp_params *ppp)
{
	return 0;
}

static int __from_user_pp_params32(struct mdp_overlay_pp_params *ppp,
				   struct mdp_overlay_pp_params32 *ppp32)
{
	__u32 data;

	if (get_user(data, &ppp32->config_ops) ||
	    put_user(data, &ppp->config_ops))
		return -EFAULT;

	return 0;
}

static int __to_user_mdp_overlay(struct mdp_overlay32 __user *ov32,
				 struct mdp_overlay __user *ov)
{
	int ret = 0;

	ret = copy_in_user(&ov32->src, &ov->src, sizeof(ov32->src)) ||
		copy_in_user(&ov32->src_rect,
			&ov->src_rect, sizeof(ov32->src_rect)) ||
		copy_in_user(&ov32->dst_rect,
			&ov->dst_rect, sizeof(ov32->dst_rect));
	if (ret)
		return -EFAULT;

	ret |= put_user(ov->z_order, &ov32->z_order);
	ret |= put_user(ov->is_fg, &ov32->is_fg);
	ret |= put_user(ov->alpha, &ov32->alpha);
	ret |= put_user(ov->blend_op, &ov32->blend_op);
	ret |= put_user(ov->transp_mask, &ov32->transp_mask);
	ret |= put_user(ov->flags, &ov32->flags);
	ret |= put_user(ov->id, &ov32->id);
	if (ret)
		return -EFAULT;

	ret = copy_in_user(&ov32->user_data, &ov->user_data,
		     sizeof(ov32->user_data));
	if (ret)
		return -EFAULT;

	ret |= put_user(ov->horz_deci, &ov32->horz_deci);
	ret |= put_user(ov->vert_deci, &ov32->vert_deci);
	if (ret)
		return -EFAULT;

	ret = __to_user_pp_params(&ov32->overlay_pp_cfg, &ov->overlay_pp_cfg);
	if (ret)
		return -EFAULT;

	ret = copy_in_user(&ov32->scale, &ov->scale,
			   sizeof(struct mdp_scale_data));
	if (ret)
		return -EFAULT;
	return 0;
}


static int __from_user_mdp_overlay(struct mdp_overlay *ov,
				   struct mdp_overlay32 __user *ov32)
{
	__u32 data;

	if (copy_in_user(&ov->src, &ov32->src,
			 sizeof(ov32->src)) ||
	    copy_in_user(&ov->src_rect, &ov32->src_rect,
			 sizeof(ov32->src_rect)) ||
	    copy_in_user(&ov->dst_rect, &ov32->dst_rect,
			 sizeof(ov32->dst_rect)))
		return -EFAULT;

	if (get_user(data, &ov32->z_order) ||
	    put_user(data, &ov->z_order) ||
	    get_user(data, &ov32->is_fg) ||
	    put_user(data, &ov->is_fg) ||
	    get_user(data, &ov32->alpha) ||
	    put_user(data, &ov->alpha) ||
	    get_user(data, &ov32->blend_op) ||
	    put_user(data, &ov->blend_op) ||
	    get_user(data, &ov32->transp_mask) ||
	    put_user(data, &ov->transp_mask) ||
	    get_user(data, &ov32->flags) ||
	    put_user(data, &ov->flags) ||
	    get_user(data, &ov32->pipe_type) ||
	    put_user(data, &ov->pipe_type) ||
	    get_user(data, &ov32->id) ||
	    put_user(data, &ov->id))
		return -EFAULT;

	if (copy_in_user(&ov->user_data, &ov32->user_data,
			 sizeof(ov32->user_data)))
		return -EFAULT;

	if (get_user(data, &ov32->horz_deci) ||
	    put_user(data, &ov->horz_deci) ||
	    get_user(data, &ov32->vert_deci) ||
	    put_user(data, &ov->vert_deci))
		return -EFAULT;

	if (__from_user_pp_params32(&ov->overlay_pp_cfg,
				    &ov32->overlay_pp_cfg))
		return -EFAULT;

	if (copy_in_user(&ov->scale, &ov32->scale,
			 sizeof(struct mdp_scale_data)))
		return -EFAULT;

	return 0;
}

static int __from_user_mdp_overlaylist(struct mdp_overlay_list *ovlist,
				   struct mdp_overlay_list32 *ovlist32,
				   struct mdp_overlay **to_list_head)
{
	__u32 i, ret;
	unsigned long data, from_list_head;
	struct mdp_overlay32 *iter;

	if (!to_list_head || !ovlist32 || !ovlist) {
		pr_err("%s:%u: null error\n", __func__, __LINE__);
		return -EINVAL;
	}

	if (copy_in_user(&ovlist->num_overlays, &ovlist32->num_overlays,
			 sizeof(ovlist32->num_overlays)))
		return -EFAULT;

	if (copy_in_user(&ovlist->flags, &ovlist32->flags,
			 sizeof(ovlist32->flags)))
		return -EFAULT;

	if (copy_in_user(&ovlist->processed_overlays,
			&ovlist32->processed_overlays,
			 sizeof(ovlist32->processed_overlays)))
		return -EFAULT;

	if (get_user(data, &ovlist32->overlay_list)) {
		ret = -EFAULT;
		goto validate_exit;
	}
	for (i = 0; i < ovlist32->num_overlays; i++) {
		if (get_user(from_list_head, (__u32 *)data + i)) {
			ret = -EFAULT;
			goto validate_exit;
		}

		iter = compat_ptr(from_list_head);
		if (__from_user_mdp_overlay(to_list_head[i],
			       (struct mdp_overlay32 *)(iter))) {
			ret = -EFAULT;
			goto validate_exit;
		}
	}
	ovlist->overlay_list = to_list_head;

	return 0;

validate_exit:
	pr_err("%s: %u: copy error\n", __func__, __LINE__);
	return -EFAULT;
}

static int __to_user_mdp_overlaylist(struct mdp_overlay_list32 *ovlist32,
				   struct mdp_overlay_list *ovlist,
				   struct mdp_overlay **l_ptr)
{
	__u32 i, ret;
	unsigned long data, data1;
	struct mdp_overlay32 *temp;
	struct mdp_overlay *l = l_ptr[0];

	if (copy_in_user(&ovlist32->num_overlays, &ovlist->num_overlays,
			 sizeof(ovlist32->num_overlays)))
		return -EFAULT;

	if (get_user(data, &ovlist32->overlay_list)) {
		ret = -EFAULT;
		pr_err("%s:%u: err\n", __func__, __LINE__);
		goto validate_exit;
	}

	for (i = 0; i < ovlist32->num_overlays; i++) {
		if (get_user(data1, (__u32 *)data + i)) {
			ret = -EFAULT;
			goto validate_exit;
		}
		temp = compat_ptr(data1);
		if (__to_user_mdp_overlay(
				(struct mdp_overlay32 *) temp,
				l + i)) {
			ret = -EFAULT;
			goto validate_exit;
		}
	}

	if (copy_in_user(&ovlist32->flags, &ovlist->flags,
				sizeof(ovlist32->flags)))
		return -EFAULT;

	if (copy_in_user(&ovlist32->processed_overlays,
			&ovlist->processed_overlays,
			sizeof(ovlist32->processed_overlays)))
		return -EFAULT;

	return 0;

validate_exit:
	pr_err("%s: %u: copy error\n", __func__, __LINE__);
	return -EFAULT;

}

void mdss_compat_align_list(void __user *total_mem_chunk,
		struct mdp_overlay __user **list_ptr, u32 num_ov)
{
	int i = 0;
	struct mdp_overlay __user *contig_overlays;

	contig_overlays = total_mem_chunk + sizeof(struct mdp_overlay_list) +
		 (num_ov * sizeof(struct mdp_overlay *));

	for (i = 0; i < num_ov; i++)
		list_ptr[i] = contig_overlays + i;
}

int mdss_compat_overlay_ioctl(struct fb_info *info, unsigned int cmd,
			 unsigned long arg)
{
	struct mdp_overlay *ov, **layers_head;
	struct mdp_overlay32 *ov32;
	struct mdp_overlay_list __user *ovlist;
	struct mdp_overlay_list32 __user *ovlist32;
	size_t layers_refs_sz, layers_sz, prepare_sz;
	void __user *total_mem_chunk;
	int ret;

	if (!info || !info->par)
		return -EINVAL;

	ov = compat_alloc_user_space(sizeof(*ov));

	switch (cmd) {
	case MSMFB_OVERLAY_GET:
		ov32 = compat_ptr(arg);
		ret = __from_user_mdp_overlay(ov, ov32);
		if (ret)
			pr_err("%s: compat mdp overlay failed\n", __func__);
		else
			ret = mdss_fb_do_ioctl(info, cmd, (unsigned long) ov);
		ret = __to_user_mdp_overlay(ov32, ov);
		break;
	case MSMFB_OVERLAY_SET:
		ov32 = compat_ptr(arg);
		ret = __from_user_mdp_overlay(ov, ov32);
		if (ret) {
			pr_err("%s: compat mdp overlay failed\n", __func__);
		} else {
			ret = mdss_fb_do_ioctl(info, cmd, (unsigned long) ov);
			ret = __to_user_mdp_overlay(ov32, ov);
		}
		break;
	case MSMFB_OVERLAY_PREPARE:
		ovlist32 = compat_ptr(arg);

		layers_sz = ovlist32->num_overlays *
					sizeof(struct mdp_overlay);
		prepare_sz = sizeof(struct mdp_overlay_list);
		layers_refs_sz = ovlist32->num_overlays *
					sizeof(struct mdp_overlay *);

		total_mem_chunk = compat_alloc_user_space(
			prepare_sz + layers_refs_sz + layers_sz);
		if (!total_mem_chunk) {
			pr_err("%s:%u: compat alloc error [%zu] bytes\n",
				 __func__, __LINE__,
				 layers_refs_sz + layers_sz + prepare_sz);
			return -EINVAL;
		}

		layers_head = total_mem_chunk + prepare_sz;
		mdss_compat_align_list(total_mem_chunk, layers_head,
					ovlist32->num_overlays);
		ovlist = (struct mdp_overlay_list *)total_mem_chunk;

		ret = __from_user_mdp_overlaylist(ovlist, ovlist32,
					layers_head);
		if (ret) {
			pr_err("compat mdp overlaylist failed\n");
		} else {
			ret = mdss_fb_do_ioctl(info, cmd,
						(unsigned long) ovlist);
			if (!ret)
				ret = __to_user_mdp_overlaylist(ovlist32,
							 ovlist, layers_head);
		}
		break;
	case MSMFB_OVERLAY_UNSET:
	case MSMFB_OVERLAY_PLAY_ENABLE:
	case MSMFB_OVERLAY_PLAY:
	case MSMFB_OVERLAY_PLAY_WAIT:
	case MSMFB_VSYNC_CTRL:
	case MSMFB_OVERLAY_VSYNC_CTRL:
	case MSMFB_OVERLAY_COMMIT:
	case MSMFB_METADATA_SET:
	case MSMFB_METADATA_GET:
	default:
		pr_debug("%s: overlay ioctl cmd=[%u]\n", __func__, cmd);
		ret = mdss_fb_do_ioctl(info, cmd, (unsigned long) arg);
		break;
	}
	return ret;
}

/*
 * mdss_fb_compat_ioctl() - MDSS Framebuffer compat ioctl function
 * @info:	pointer to framebuffer info
 * @cmd:	ioctl command
 * @arg:	argument to ioctl
 *
 * This function adds the compat translation layer for framebuffer
 * ioctls to allow 32-bit userspace call ioctls on the mdss
 * framebuffer device driven in 64-bit kernel.
 */
int mdss_fb_compat_ioctl(struct fb_info *info, unsigned int cmd,
			 unsigned long arg)
{
	int ret;

	if (!info || !info->par)
		return -EINVAL;

	cmd = __do_compat_ioctl_nr(cmd);
	switch (cmd) {
	case MSMFB_CURSOR:
		pr_debug("%s: MSMFB_CURSOR not supported\n", __func__);
		ret = -ENOSYS;
		break;
	case MSMFB_SET_LUT:
		ret = mdss_fb_compat_set_lut(info, arg);
		break;
	case MSMFB_BUFFER_SYNC:
		ret = mdss_fb_compat_buf_sync(info, cmd, arg);
		break;
	case MSMFB_OVERLAY_GET:
	case MSMFB_OVERLAY_SET:
	case MSMFB_OVERLAY_UNSET:
	case MSMFB_OVERLAY_PLAY_ENABLE:
	case MSMFB_OVERLAY_PLAY:
	case MSMFB_OVERLAY_PLAY_WAIT:
	case MSMFB_VSYNC_CTRL:
	case MSMFB_OVERLAY_VSYNC_CTRL:
	case MSMFB_OVERLAY_COMMIT:
	case MSMFB_METADATA_SET:
	case MSMFB_METADATA_GET:
	case MSMFB_OVERLAY_PREPARE:
		ret = mdss_compat_overlay_ioctl(info, cmd, arg);
		break;
	case MSMFB_NOTIFY_UPDATE:
	case MSMFB_DISPLAY_COMMIT:
	default:
		ret = mdss_fb_do_ioctl(info, cmd, arg);
		break;
	}

	if (ret == -ENOSYS)
		pr_err("%s: unsupported ioctl\n", __func__);
	else if (ret)
		pr_debug("%s: ioctl err cmd=%u ret=%d\n", __func__, cmd, ret);

	return ret;
}
EXPORT_SYMBOL(mdss_fb_compat_ioctl);
