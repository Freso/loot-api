/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2013-2016    WrinklyNinja

    This file is part of LOOT.

    LOOT is free software: you can redistribute
    it and/or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version.

    LOOT is distributed in the hope that it will
    be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with LOOT.  If not, see
    <https://www.gnu.org/licenses/>.
    */

#ifndef LOOT_API_H
#define LOOT_API_H

#include <string>
#include <memory>

#include "loot/api_decorator.h"
#include "loot/game_interface.h"
#include "loot/exception/error_categories.h"
#include "loot/exception/condition_syntax_error.h"
#include "loot/exception/cyclic_interaction_error.h"
#include "loot/exception/file_access_error.h"
#include "loot/exception/git_state_error.h"
#include "loot/enum/game_type.h"
#include "loot/enum/log_verbosity.h"
#include "loot/loot_version.h"

namespace loot {
/**@}*/
/**********************************************************************//**
 *  @name Logging Functions
 *************************************************************************/
/**@{*/

/**
 * @brief Set the API's logging verbosity.
 * @details The default is ``LogVerbosity::off``.
 * @param verbosity
 *        The logging verbosity to set.
 */
LOOT_API void SetLoggingVerbosity(LogVerbosity verbosity);

/**
 * @brief Set the file path that logging statements are written to.
 * @details If no file is set the default behaviour is to print logging
 *          statements to the console.
 * @param path
 *        The log file path.
 */
LOOT_API void SetLogFile(const std::string& path);

/**@}*/
/**********************************************************************//**
 *  @name Version Functions
 *************************************************************************/
/**@{*/

/**
 *  @brief Checks for API compatibility.
 *  @details Checks whether the loaded API is compatible with the given
 *           version of the API, abstracting API stability policy away from
 *           clients. The version numbering used is major.minor.patch.
 *  @param major
 *         The major version number to check.
 *  @param minor
 *         The minor version number to check.
 *  @param patch
 *         The patch version number to check.
 *  @returns True if the API versions are compatible, false otherwise.
 */
LOOT_API bool IsCompatible(const unsigned int major,
                           const unsigned int minor,
                           const unsigned int patch);

/**@}*/
/**********************************************************************//**
 *  @name Lifecycle Management Functions
 *************************************************************************/
/**@{*/

/**
 * Initialise the current global locale using the given ID.
 *
 * This sets the global locale up so that the library's UTF-8 support can
 * function.
 * @param id A locale ID.
 */
LOOT_API void InitialiseLocale(const std::string& id);

/**
 *  @brief Initialise a new game handle.
 *  @details Creates a handle for a game, which is then used by all
 *           game-specific functions.
 *  @param game
 *         A game code for which to create the handle.
 *  @param game_path
 *         The relative or absolute path to the game folder, or an empty string.
 *         If an empty string, the API will attempt to detect the data path of
 *         the specified game by searching for the game's main master file in a
 *         sibling Data folder and by searching for the game's Registry entry.
 *  @param game_local_path
 *         The relative or absolute path to the game's folder in
 *         `%%LOCALAPPDATA%` or an empty string. If an empty string, the API will
 *         attempt to look up the path that `%%LOCALAPPDATA%` corresponds to.
 *         This parameter is provided so that systems lacking that environmental
 *         variable (eg. Linux) can still use the API.
 *  @returns The new game handle.
 */
LOOT_API std::shared_ptr<GameInterface> CreateGameHandle(const GameType game,
                                                         const std::string& game_path = "",
                                                         const std::string& game_local_path = "");
}

#endif
