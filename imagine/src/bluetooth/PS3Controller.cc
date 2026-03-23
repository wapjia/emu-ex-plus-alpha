/*  This file is part of Imagine.

	Imagine is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Imagine is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Imagine.  If not, see <http://www.gnu.org/licenses/> */

#include <imagine/bluetooth/PS3Controller.hh>
#include <imagine/input/bluetoothInputDefs.hh>
#include <imagine/base/Application.hh>
#include <imagine/util/ranges.hh>
#include <imagine/util/variant.hh>
#include <imagine/logger/SystemLogger.hh>
import std;
import packedInputAccess;

constexpr std::uint32_t CELL_PAD_BTN_OFFSET_DIGITAL1 = 0, CELL_PAD_BTN_OFFSET_DIGITAL2 = 1;

// CELL_PAD_BTN_OFFSET_DIGITAL1
#define CELL_PAD_CTRL_LEFT      (1 << 7)
#define CELL_PAD_CTRL_DOWN      (1 << 6)
#define CELL_PAD_CTRL_RIGHT     (1 << 5)
#define CELL_PAD_CTRL_UP        (1 << 4)
#define CELL_PAD_CTRL_START     (1 << 3)
#define CELL_PAD_CTRL_R3        (1 << 2)
#define CELL_PAD_CTRL_L3        (1 << 1)
#define CELL_PAD_CTRL_SELECT    (1 << 0)

// CELL_PAD_BTN_OFFSET_DIGITAL2
#define CELL_PAD_CTRL_SQUARE    (1 << 7)
#define CELL_PAD_CTRL_CROSS     (1 << 6)
#define CELL_PAD_CTRL_CIRCLE    (1 << 5)
#define CELL_PAD_CTRL_TRIANGLE  (1 << 4)
#define CELL_PAD_CTRL_R1        (1 << 3)
#define CELL_PAD_CTRL_L1        (1 << 2)
#define CELL_PAD_CTRL_R2        (1 << 1)
#define CELL_PAD_CTRL_L2        (1 << 0)

#define CELL_PAD_CTRL_PS        (1 << 0)

namespace IG
{

using namespace IG::Input;

static SystemLogger log{"PS3Ctrl"};

static const PackedInputAccess padDataAccess[] =
{
	{ CELL_PAD_BTN_OFFSET_DIGITAL1, CELL_PAD_CTRL_SELECT, PS3BtKey::SELECT },
	{ CELL_PAD_BTN_OFFSET_DIGITAL1, CELL_PAD_CTRL_L3, PS3BtKey::L3 },
	{ CELL_PAD_BTN_OFFSET_DIGITAL1, CELL_PAD_CTRL_R3, PS3BtKey::R3 },
	{ CELL_PAD_BTN_OFFSET_DIGITAL1, CELL_PAD_CTRL_START, PS3BtKey::START },
	{ CELL_PAD_BTN_OFFSET_DIGITAL1, CELL_PAD_CTRL_UP, PS3BtKey::UP },
	{ CELL_PAD_BTN_OFFSET_DIGITAL1, CELL_PAD_CTRL_RIGHT, PS3BtKey::RIGHT },
	{ CELL_PAD_BTN_OFFSET_DIGITAL1, CELL_PAD_CTRL_DOWN, PS3BtKey::DOWN },
	{ CELL_PAD_BTN_OFFSET_DIGITAL1, CELL_PAD_CTRL_LEFT, PS3BtKey::LEFT },

	{ CELL_PAD_BTN_OFFSET_DIGITAL2, CELL_PAD_CTRL_L2, PS3BtKey::L2 },
	{ CELL_PAD_BTN_OFFSET_DIGITAL2, CELL_PAD_CTRL_R2, PS3BtKey::R2 },
	{ CELL_PAD_BTN_OFFSET_DIGITAL2, CELL_PAD_CTRL_L1, PS3BtKey::L1 },
	{ CELL_PAD_BTN_OFFSET_DIGITAL2, CELL_PAD_CTRL_R1, PS3BtKey::R1 },
	{ CELL_PAD_BTN_OFFSET_DIGITAL2, CELL_PAD_CTRL_TRIANGLE, PS3BtKey::TRIANGLE },
	{ CELL_PAD_BTN_OFFSET_DIGITAL2, CELL_PAD_CTRL_CIRCLE, PS3BtKey::CIRCLE },
	{ CELL_PAD_BTN_OFFSET_DIGITAL2, CELL_PAD_CTRL_CROSS, PS3BtKey::CROSS },
	{ CELL_PAD_BTN_OFFSET_DIGITAL2, CELL_PAD_CTRL_SQUARE, PS3BtKey::SQUARE },

	{ CELL_PAD_BTN_OFFSET_DIGITAL2+1, CELL_PAD_CTRL_PS, PS3BtKey::PS },
};

static const char *ps3ButtonName(Input::Key k)
{
	using namespace IG::Input;
	switch(k)
	{
		case 0: return "None";
		case PS3BtKey::CROSS: return "Cross";
		case PS3BtKey::CIRCLE: return "Circle";
		case PS3BtKey::SQUARE: return "Square";
		case PS3BtKey::TRIANGLE: return "Triangle";
		case PS3BtKey::L1: return "L1";
		case PS3BtKey::L2: return "L2";
		case PS3BtKey::L3: return "L3";
		case PS3BtKey::R1: return "R1";
		case PS3BtKey::R2: return "R2";
		case PS3BtKey::R3: return "R3";
		case PS3BtKey::SELECT: return "Select";
		case PS3BtKey::START: return "Start";
		case PS3BtKey::UP: return "Up";
		case PS3BtKey::RIGHT: return "Right";
		case PS3BtKey::DOWN: return "Down";
		case PS3BtKey::LEFT: return "Left";
		case PS3BtKey::PS: return "PS";
		case PS3BtKey::LSTICK_UP: return "L:Up";
		case PS3BtKey::LSTICK_RIGHT: return "L:Right";
		case PS3BtKey::LSTICK_DOWN: return "L:Down";
		case PS3BtKey::LSTICK_LEFT: return "L:Left";
		case PS3BtKey::RSTICK_UP: return "R:Up";
		case PS3BtKey::RSTICK_RIGHT: return "R:Right";
		case PS3BtKey::RSTICK_DOWN: return "R:Down";
		case PS3BtKey::RSTICK_LEFT: return "R:Left";
	}
	return "";
}

PS3Controller::PS3Controller(ApplicationContext ctx, BluetoothAddr):
	BluetoothInputDevice{ctx, Input::Map::PS3PAD, {.gamepad = true}, "PS3 Controller"},
	ctlSock{ctx}, intSock{ctx} {}

const char *PS3Controller::keyName(Input::Key k) const
{
	return ps3ButtonName(k);
}

bool PS3Controller::open(BluetoothAdapter&, Input::Device&)
{
	return false;
}

bool PS3Controller::open1Ctl(BluetoothAdapter &adapter, BluetoothPendingSocket &pending, Input::Device &dev)
{
	ctlSock.onData = intSock.onData =
		[&dev](const char *packet, size_t size)
		{
			return getAs<PS3Controller>(dev).dataHandler(dev, packet, size);
		};
	ctlSock.onStatus = intSock.onStatus =
		[&dev](BluetoothSocket &sock, BluetoothSocketState status)
		{
			return getAs<PS3Controller>(dev).statusHandler(dev, sock, status);
		};
	log.info("accepting PS3 control channel");
	if(auto err = ctlSock.open(adapter, pending);
		err.code())
	{
		log.error("error opening control socket");
		return false;
	}
	return true;
}

bool PS3Controller::open2Int(BluetoothAdapter &adapter, BluetoothPendingSocket &pending)
{
	log.info("accepting PS3 interrupt channel");
	if(auto err = intSock.open(adapter, pending);
		err.code())
	{
		log.error("error opening interrupt socket");
		return false;
	}
	return true;
}

uint32_t PS3Controller::statusHandler(Input::Device &dev, BluetoothSocket &sock, BluetoothSocketState status)
{
	if(status == BluetoothSocketState::Opened && &sock == (BluetoothSocket*)&ctlSock)
	{
		log.info("opened PS3 control socket, waiting for interrupt socket");
		return 0; // don't add ctlSock to event loop
	}
	else if(status == BluetoothSocketState::Opened && &sock == (BluetoothSocket*)&intSock)
	{
		log.info("PS3 controller opened successfully");
		ctx.application().bluetoothInputDeviceStatus(ctx, dev, status);
		sendFeatureReport();
		return 1;
	}
	else if(status == BluetoothSocketState::ConnectError)
	{
		log.error("PS3 controller connection error");
		ctx.application().bluetoothInputDeviceStatus(ctx, dev, status);
	}
	else if(status == BluetoothSocketState::ReadError)
	{
		log.error("PS3 controller read error, disconnecting");
		ctx.application().bluetoothInputDeviceStatus(ctx, dev, status);
	}
	return 1;
}

void PS3Controller::close()
{
	intSock.close();
	ctlSock.close();
}

bool PS3Controller::dataHandler(Input::Device& dev, const char* packetPtr, size_t)
{
	auto packet = (const uint8_t*)packetPtr;
	/*log.info("data with size %d", (int)size);
	iterateTimes(size, i)
	{
		logger_printf(0, "0x%X ", packet[i]);
	}
	if(size)
		logger_printf(0, "\n");*/
	if(!didSetLEDs) [[unlikely]]
	{
		setLEDs(dev.enumId());
		didSetLEDs = true;
	}

	switch(packet[0])
	{
		case 0xA1:
		{
			auto time = SteadyClock::now();
			const uint8_t *digitalBtnData = &packet[3];
			for(auto &e : padDataAccess)
			{
				int newState = e.updateState(prevData, digitalBtnData);
				if(newState != -1)
				{
					//log.info("{} {} @ PS3 Pad %d", device->keyName(e.keyEvent), newState ? "pushed" : "released", player);
					ctx.endIdleByUserActivity();
					KeyEvent event{Map::PS3PAD, e.key, newState ? Action::PUSHED : Action::RELEASED, 0, 0, Source::GAMEPAD, time, &dev};
					ctx.application().dispatchRepeatableKeyInputEvent(event);
				}
			}
			std::memcpy(prevData, digitalBtnData, sizeof(prevData));

			const uint8_t *stickData = &packet[7];
			//log.info("left: {},{} right: {},{}", stickData[0], stickData[1], stickData[2], stickData[3]);
			for(auto i: iotaCount(4))
			{
				if(axis[i].dispatchInputEvent(int(stickData[i]) - 127, Map::PS3PAD, time, dev, ctx.mainWindow()))
					ctx.endIdleByUserActivity();
			}
		}
	}

	return 1;
}

static constexpr uint32_t HIDP_TRANSACTION_SET_REPORT = 0x50;
static constexpr uint32_t HIDP_DATA_HEADER_RTYPE_OUTPUT = 0x02;
static constexpr uint32_t HIDP_DATA_HEADER_RTYPE_FEATURE = 0x03;

void PS3Controller::sendFeatureReport()
{
	log.info("sending feature report");
	const uint8_t featureReport[]
	{
		HIDP_TRANSACTION_SET_REPORT | HIDP_DATA_HEADER_RTYPE_FEATURE,
		0xf4, 0x42, 0x03, 0x00, 0x00
	};
	ctlSock.write(featureReport, sizeof(featureReport));
}

void PS3Controller::setLEDs(uint32_t player)
{
	log.info("setting LEDs for player:{}", player);
	uint8_t setLEDs[] =
	{
		HIDP_TRANSACTION_SET_REPORT | HIDP_DATA_HEADER_RTYPE_OUTPUT,
		0x01,
		0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00,
		0xff, 0x27, 0x10, 0x00, 0x32,
		0xff, 0x27, 0x10, 0x00, 0x32,
		0xff, 0x27, 0x10, 0x00, 0x32,
		0xff, 0x27, 0x10, 0x00, 0x32,
		0x00, 0x00, 0x00, 0x00, 0x00
	};
	setLEDs[11] = playerLEDs(player);
	ctlSock.write(setLEDs, sizeof(setLEDs));
}

uint8_t PS3Controller::playerLEDs(uint32_t player)
{
	switch(player)
	{
		default:
		case 0: return bit(1);
		case 1: return bit(2);
		case 2: return bit(3);
		case 3: return bit(4);
		case 4: return bit(4) | bit(1);
	}
}

}
