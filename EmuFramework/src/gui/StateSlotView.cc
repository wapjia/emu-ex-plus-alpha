/*  This file is part of EmuFramework.

	Imagine is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Imagine is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with EmuFramework.  If not, see <http://www.gnu.org/licenses/> */

#include <emuframework/StateSlotView.hh>
#include <emuframework/EmuApp.hh>
import imagine;

namespace EmuEx
{

constexpr SystemLogger log{"StateSlotView"};

static auto slotHeadingName(EmuApp& app)
{
	return std::format("Set State Slot ({})", app.stateSlot());
}

StateSlotView::StateSlotView(ViewAttachParams attach):
	TableView{"Save States", attach, menuItems},
	load
	{
		"Load State", attach,
		[this](TextMenuItem &item, View &, const Input::Event &e)
		{
			if(!item.active())
				return;
			pushAndShowModal(makeView<YesNoAlertView>("Really load state?",
				YesNoAlertView::Delegates
				{
					.onYes = [this]
					{
						auto& app = this->app();
						if(app.loadStateWithSlot(app.stateSlot()))
							app.showEmulation();
					}
				}), e);
		}
	},
	save
	{
		"Save State", attach,
		[this](const Input::Event &e)
		{
			if(app().shouldOverwriteExistingState())
			{
				doSaveState();
			}
			else
			{
				pushAndShowModal(makeView<YesNoAlertView>("Really overwrite state?",
					YesNoAlertView::Delegates{.onYes = [this]{ doSaveState(); }}), e);
			}
		}
	},
	slotHeading{slotHeadingName(app()), attach},
	menuItems
	{
		&load, &save, &slotHeading,
		&stateSlot[0], &stateSlot[1], &stateSlot[2], &stateSlot[3], &stateSlot[4],
		&stateSlot[5], &stateSlot[6], &stateSlot[7], &stateSlot[8], &stateSlot[9]
	}
{
	assume(system().hasContent());
	refreshSlots();
}

void StateSlotView::onShow()
{
	refreshSlots();
	place();
}

void StateSlotView::refreshSlot(int slot)
{
	auto &app = this->app();
	auto &sys = app.system();
	auto saveStr = sys.statePath(slot);
	auto modTimeStr = appContext().fileUriFormatLastWriteTimeLocal(saveStr);
	bool fileExists = modTimeStr.size();
	auto str = [&]()
	{
		if(fileExists)
			return std::format("{} ({})", sys.stateSlotName(slot), modTimeStr);
		else
			return std::format("{}", sys.stateSlotName(slot));
	};
	auto &s = stateSlot[slot];
	s = {str(), attachParams(), [this, slot](View&)
	{
		auto &app = this->app();
		auto &sys = app.system();
		stateSlot[app.stateSlot()].setHighlighted(false);
		stateSlot[slot].setHighlighted(true);
		app.setStateSlot(slot);
		log.info("set state slot:{}", app.stateSlot());
		slotHeading.compile(slotHeadingName(app));
		load.setActive(sys.stateExists(app.stateSlot()));
		postDraw();
	}};
	if(slot == app.stateSlot())
		load.setActive(fileExists);
}

void StateSlotView::refreshSlots()
{
	for(auto i: iotaCount(stateSlots))
	{
		refreshSlot(i);
	}
	stateSlot[app().stateSlot()].setHighlighted(true);
}

void StateSlotView::doSaveState()
{
	auto& app = this->app();
	auto slot = app.stateSlot();
	if(app.saveStateWithSlot(slot, false))
		app.showEmulation();
	refreshSlot(slot);
	place();
}

}
