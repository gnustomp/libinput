/*
 * Copyright © 2016 James Ye <jye836@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <config.h>

#include <check.h>
#include <errno.h>
#include <fcntl.h>
#include <libinput.h>
#include <unistd.h>

#include "libinput-util.h"
#include "litest.h"

START_TEST(lid_open_close)
{
	struct litest_device *sw = litest_current_device();
	struct libinput *li = sw->libinput;
	struct libinput_event *event;

	litest_drain_events(li);

	/* lid closed */
	litest_lid_action(sw, LIBINPUT_SWITCH_STATE_ON);
	libinput_dispatch(li);

	event = libinput_get_event(li);
	litest_is_switch_event(event, LIBINPUT_SWITCH_STATE_ON);

	libinput_event_destroy(event);

	/* lid opened */
	litest_lid_action(sw, LIBINPUT_SWITCH_STATE_OFF);
	libinput_dispatch(li);

	event = libinput_get_event(li);
	litest_is_switch_event(event, LIBINPUT_SWITCH_STATE_OFF);

	libinput_event_destroy(event);
}
END_TEST

static inline struct litest_device *
lid_init_paired_touchpad(struct libinput *li)
{
	enum litest_device_type which = LITEST_SYNAPTICS_I2C;

	return litest_add_device(li, which);
}

START_TEST(lid_disable_touchpad)
{
	struct litest_device *sw = litest_current_device();
	struct litest_device *touchpad;
	struct libinput *li = sw->libinput;

	touchpad = lid_init_paired_touchpad(li);
	litest_disable_tap(touchpad->libinput_device);
	litest_drain_events(li);

	litest_lid_action(sw, LIBINPUT_SWITCH_STATE_ON);
	libinput_dispatch(li);
	litest_touch_down(touchpad, 0, 50, 50);
	litest_touch_move_to(touchpad, 0, 50, 50, 70, 50, 10, 1);
	litest_touch_up(touchpad, 0);

	litest_assert_only_typed_events(li, LIBINPUT_EVENT_SWITCH_TOGGLE);

	/* lid is down - no events */
	litest_touch_down(touchpad, 0, 50, 50);
	litest_touch_move_to(touchpad, 0, 50, 50, 70, 50, 10, 1);
	litest_touch_up(touchpad, 0);

	litest_assert_empty_queue(li);

	/* lid is up - motion events */
	litest_lid_action(sw, LIBINPUT_SWITCH_STATE_OFF);
	libinput_dispatch(li);
	litest_drain_events(li);

	litest_touch_down(touchpad, 0, 50, 50);
	litest_touch_move_to(touchpad, 0, 50, 50, 70, 50, 10, 1);
	litest_touch_up(touchpad, 0);

	litest_assert_only_typed_events(li, LIBINPUT_EVENT_POINTER_MOTION);

	litest_delete_device(touchpad);
}
END_TEST

void
litest_setup_tests_lid(void)
{
	litest_add("lid:lid_open_close", lid_open_close, LITEST_SWITCH, LITEST_ANY);
	litest_add("lid:lid_disable_touchpad", lid_disable_touchpad, LITEST_SWITCH, LITEST_ANY);
}
