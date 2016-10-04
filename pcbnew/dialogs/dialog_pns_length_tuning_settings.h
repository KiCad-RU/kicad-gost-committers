/*
 * KiRouter - a push-and-(sometimes-)shove PCB router
 *
 * Copyright (C) 2014  CERN
 * Copyright (C) 2016 KiCad Developers, see AUTHORS.txt for contributors.
 * Author: Maciej Suminski <maciej.suminski@cern.ch>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.or/licenses/>.
 */

/**
 * Push and Shove router settings dialog.
 */

#ifndef __dialog_pns_length_tuning_settings__
#define __dialog_pns_length_tuning_settings__

#include "dialog_pns_length_tuning_settings_base.h"

#include <wx_unit_binder.h>

#include <router/pns_router.h>

namespace PNS {

class MEANDER_SETTINGS;

}

class DIALOG_PNS_LENGTH_TUNING_SETTINGS : public DIALOG_PNS_LENGTH_TUNING_SETTINGS_BASE
{
public:
    DIALOG_PNS_LENGTH_TUNING_SETTINGS( wxWindow* aParent, PNS::MEANDER_SETTINGS& aSettings, PNS::ROUTER_MODE aMode );

    virtual void OnOkClick( wxCommandEvent& aEvent ) override;

private:
    WX_UNIT_BINDER m_minAmpl;
    WX_UNIT_BINDER m_maxAmpl;
    WX_UNIT_BINDER m_spacing;
    WX_UNIT_BINDER m_targetLength;

    PNS::MEANDER_SETTINGS& m_settings;
    PNS::ROUTER_MODE m_mode;
};

#endif // __dialog_pns_settings__
