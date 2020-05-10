// coding: utf-8
/*
 * Copyright (c) 2017-2018, Niklas Hauser
 * Copyright (c) 2020, Ethan Slattery
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
// ----------------------------------------------------------------------------

#ifndef MODM_STM32_SENSORTILE_L476JGY_HPP
#define MODM_STM32_SENSORTILE_L476JGY_HPP

#include <modm/platform.hpp>
#include <modm/architecture/interface/clock.hpp>
//#include <modm/debug/logger.hpp>
/// @ingroup modm_board_nucleo_l476rg
//#define MODM_BOARD_HAS_LOGGER

using namespace modm::platform;

/// @ingroup modm_board_nucleo_l476rg
namespace Board
{
	using namespace modm::literals;

/// STM32L4 running at 80MHz generated from the
/// internal Multispeed oscillator

// Dummy clock for devices
struct SystemClock {
	static constexpr uint32_t Frequency = 80_MHz;
	static constexpr uint32_t Ahb  = Frequency;
	static constexpr uint32_t Apb1 = Frequency;
	static constexpr uint32_t Apb2 = Frequency;

	static constexpr uint32_t I2c1 = Apb1;
	static constexpr uint32_t I2c2 = Apb1;
	static constexpr uint32_t I2c3 = Apb1;

	static constexpr uint32_t Spi1 = Apb2;

	static constexpr uint32_t Spi2 = Apb1;
	static constexpr uint32_t Spi3 = Apb1;

	static constexpr uint32_t Can1 = Apb1;

	static constexpr uint32_t Usart1 = Apb2;

	static constexpr uint32_t Usart2 = Apb1;
	static constexpr uint32_t Usart3 = Apb1;
	static constexpr uint32_t Usart4 = Apb1;
	static constexpr uint32_t Usart5 = Apb1;

	static bool inline
	enable()
	{
		// set flash latency first because system already runs from MSI
		Rcc::setFlashLatency<Frequency>();
		Rcc::enableMultiSpeedInternalClock(Rcc::MsiFrequency::MHz48);
		Rcc::updateCoreFrequency<Frequency>();

		return true;
	}
};

// User LD
using LedRed = GpioOutputG12;
using Leds = SoftwareGpioPort< LedRed >;

inline void
initialize()
{
	SystemClock::enable();
	SysTickTimer::initialize<SystemClock>();

	// expansion of macro __HAL_RCC_PWR_CLK_ENABLE()
	RCC->APB1ENR1 |= RCC_APB1ENR1_PWREN;
	uint32_t tmpreg = RCC->APB1ENR1 & RCC_APB1ENR1_PWREN; // Delay after an RCC peripheral clock enabling

	// expansion of HAL_PWREx_EnableVddIO2()
	PWR->CR2 |= PWR_CR2_IOSV;

	LedRed::setOutput(modm::Gpio::Low);
}

} // Board namespace

#endif	// MODM_STM32_SENSORTILE_L476JGY_HPP
