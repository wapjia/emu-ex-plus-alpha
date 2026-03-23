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

#include <imagine/base/Sensor.hh>
#include <imagine/base/ApplicationContext.hh>
#include <imagine/util/enum.hh>
#include <imagine/util/utility.hh>
#include <imagine/logger/SystemLogger.hh>
#include <android/sensor.h>

namespace IG
{

static SystemLogger log{"Sensor"};

// verify SensorType maps to Sensor.TYPE_*
static_assert(to_underlying(SensorType::Accelerometer) == 1);
static_assert(to_underlying(SensorType::Gyroscope) == 4);
static_assert(to_underlying(SensorType::Light) == 5);

SensorListener::SensorListener(ApplicationContext, SensorType type, SensorChangedDelegate del):
	AndroidSensorListener{ASensorManager_getInstance(), type, del} {}

AndroidSensorListener::AndroidSensorListener(ASensorManager *manager, SensorType type, SensorChangedDelegate changedDel):
	ctrl{std::make_unique<ControlBlock>(ControlBlock{nullptr, changedDel})}
{
	auto sensor = ASensorManager_getDefaultSensor(manager, (int)type);
	if(!sensor)
		return;
	ctrl->queue.reset(ASensorManager_createEventQueue(manager,
		ALooper_forThread(), ALOOPER_POLL_CALLBACK,
		[]([[maybe_unused]] int fd, [[maybe_unused]] int ev, void *data) -> int
		{
			auto &[queue, del] = *((ControlBlock*)data);
			ssize_t eventsRead{};
			std::array<ASensorEvent, 8> events;
			while((eventsRead = ASensorEventQueue_getEvents(queue.get(), events.data(), events.size())) > 0)
			{
				auto &lastEvent = events[eventsRead - 1];
				del(SensorValues{lastEvent.data[0], lastEvent.data[1], lastEvent.data[2]});
			}
			return 1;
		}, ctrl.get()));
	log.info("created sensor:{} event queue:{}", enumName(type), (void*)ctrl->queue.get());
	ASensorEventQueue_enableSensor(ctrl->queue.get(), sensor);
	ASensorEventQueue_setEventRate(ctrl->queue.get(), sensor, 20000);
}

void destroyASensorEventQueue(ASensorEventQueue *queue)
{
	ASensorManager_destroyEventQueue(ASensorManager_getInstance(), queue);
	log.info("destroyed sensor event queue:{}", (void*)queue);
}

}
