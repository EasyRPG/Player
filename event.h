/* event.h, routines for the event management.
   Copyright (C) 2007 EasyRPG Project <http://easyrpg.sourceforge.net/>.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */
   
#ifndef EVENT_H
#define EVENT_H

    // =========================================================================
    #include <stdlib.h>
    #include <stdio.h>
    #include <string>
    #include "tools.h"
    // =========================================================================
    
    // --- Create list of chunks opcodes ---------------------------------------
    
    struct EventPage
    {
        int CharsetToUse;
        int ChipsetToUse;
    };
    
    // --- Event structure -----------------------------------------------------
    struct Event
    {
        unsigned long ID;
        string        Name;
        int           x, y;
        
        // ---------------------------------------------------------------------
        void 
        
    };
