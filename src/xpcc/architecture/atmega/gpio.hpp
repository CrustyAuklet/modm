// coding: utf-8
// ----------------------------------------------------------------------------
/* Copyright (c) 2009, Roboterclub Aachen e.V.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Roboterclub Aachen e.V. nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ROBOTERCLUB AACHEN E.V. ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL ROBOTERCLUB AACHEN E.V. BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $Id$
 */
// ----------------------------------------------------------------------------

#ifndef XPCC__ATMEGA_GPIO_HPP
#define XPCC__ATMEGA_GPIO_HPP

#include <avr/io.h>

#include <xpcc/utils/macros.hpp>
#include <xpcc/utils/misc.hpp>
#include <xpcc/architecture/general/gpio.hpp>

namespace xpcc
{
	namespace gpio
	{
		typedef enum
		{
			NORMAL = 0,
			PULLUP = 1,
		} Configuration;
	}
}

/**
 * \brief	Create a input/output pin type
 * \ingroup	gpio
 */
#define	GPIO__IO(name, port, pin) \
	struct name { \
		ALWAYS_INLINE static void setOutput() { DDR ## port |= (1 << pin); } \
		ALWAYS_INLINE static void setInput() { DDR ## port &= ~(1 << pin); } \
		ALWAYS_INLINE static void high() { PORT ## port |= (1 << pin); } \
		ALWAYS_INLINE static void low() { PORT ## port &= ~(1 << pin); } \
		ALWAYS_INLINE static bool read() { return (PIN ## port & (1 << pin)); } \
		\
		ALWAYS_INLINE static void \
		write(bool status) { \
			if (status) { \
				high(); \
			} \
			else { \
				low(); \
			} \
		} \
	}

/**
 * \brief	Create a output pin type
 * \ingroup	gpio
 */
#define	GPIO__OUTPUT(name, port, pin) \
	struct name { \
		ALWAYS_INLINE static void setOutput() { DDR ## port |= (1 << pin); } \
		ALWAYS_INLINE static void high() { PORT ## port |= (1 << pin); } \
		ALWAYS_INLINE static void low() { PORT ## port &= ~(1 << pin); } \
		ALWAYS_INLINE static void toggle() { PORT ## port ^= (1 << pin); } \
		\
		ALWAYS_INLINE static void \
		write(bool status) { \
			if (status) { \
				high(); \
			} \
			else { \
				low(); \
			} \
		} \
	}

/**
 * \brief	Create a input type
 * \ingroup	gpio
 */
#define GPIO__INPUT(name, port, pin) \
	struct name { \
		ALWAYS_INLINE static void \
		configure(::xpcc::gpio::Configuration config = ::xpcc::gpio::NORMAL) { \
			setInput(); \
			if (config == ::xpcc::gpio::PULLUP) { \
				PORT ## port |= (1 << pin); \
			} \
			else { \
				PORT ## port &= ~(1 << pin); \
			} \
		} \
		\
		ALWAYS_INLINE static void setInput() { DDR ## port &= ~(1 << pin); } \
		ALWAYS_INLINE static bool read() { return (PIN ## port & (1 << pin)); } \
	}

/**
 * \brief	Connect the lower four bits to a nibble
 * 
 * \see		xpcc::gpio::Nibble()
 * \ingroup	gpio
 */
#define GPIO__NIBBLE_LOW(name, port) \
	struct name { \
		ALWAYS_INLINE static void setOutput() { \
			DDR ## port |= 0x0f; \
		} \
		ALWAYS_INLINE static void setInput() { \
			DDR ## port &= ~0x0f; \
		} \
		ALWAYS_INLINE static uint8_t read() { \
			return (PIN ## port & 0x0f); \
		} \
		ALWAYS_INLINE static void write(uint8_t data) { \
			PORT ## port = (data & 0x0f) | (PORT ## port & 0xf0); \
		} \
	}

/**
 * \brief	Connect the higher four bits to a nibble
 * 
 * \see		xpcc::gpio::Nibble()
 * \ingroup	gpio
 */
#define GPIO__NIBBLE_HIGH(name, port) \
	struct name { \
		ALWAYS_INLINE static void setOutput() { \
			DDR ## port |= 0xf0; \
		} \
		ALWAYS_INLINE static void setInput() { \
			DDR ## port &= ~0xf0; \
		} \
		ALWAYS_INLINE static uint8_t read() { \
			uint8_t data = PIN ## port; \
			return (data >> 4); \
		} \
		ALWAYS_INLINE static void write(uint8_t data) { \
			data = ::xpcc::utils::swap(data); \
			PORT ## port = (data & 0xf0) | (PORT ## port & 0x0f); \
		} \
	}

#endif // XPCC__ATMEGA_GPIO_HPP
