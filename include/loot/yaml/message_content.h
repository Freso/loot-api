/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2012-2016    WrinklyNinja

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
#ifndef LOOT_YAML_MESSAGE_CONTENT
#define LOOT_YAML_MESSAGE_CONTENT

#define YAML_CPP_SUPPORT_MERGE_KEYS

#include <string>

#include <yaml-cpp/yaml.h>

#include "loot/metadata/message_content.h"

namespace YAML {
template<>
struct convert<loot::MessageContent> {
  static Node encode(const loot::MessageContent& rhs) {
    Node node;
    node["text"] = rhs.GetText();
    node["lang"] = rhs.GetLanguage();

    return node;
  }

  static bool decode(const Node& node, loot::MessageContent& rhs) {
    if (!node.IsMap())
      throw RepresentationException(node.Mark(), "bad conversion: 'message content' object must be a map");
    if (!node["text"])
      throw RepresentationException(node.Mark(), "bad conversion: 'text' key missing from 'message content' object");
    if (!node["lang"])
      throw RepresentationException(node.Mark(), "bad conversion: 'lang' key missing from 'message content' object");

    std::string text = node["text"].as<std::string>();
    std::string lang = node["lang"].as<std::string>();

    rhs = loot::MessageContent(text, lang);

    return true;
  }
};

inline Emitter& operator << (Emitter& out, const loot::MessageContent& rhs) {
  out << BeginMap;

  out << Key << "lang" << Value << rhs.GetLanguage();

  out << Key << "text" << Value << YAML::SingleQuoted << rhs.GetText();

  out << EndMap;

  return out;
}
}

#endif
