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
#ifndef LOOT_YAML_MESSAGE
#define LOOT_YAML_MESSAGE

#define YAML_CPP_SUPPORT_MERGE_KEYS

#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <yaml-cpp/yaml.h>

#include "loot/metadata/message.h"

namespace YAML {
template<>
struct convert<loot::Message> {
  static Node encode(const loot::Message& rhs) {
    Node node;
    node["content"] = rhs.GetContent();

    if (rhs.GetType() == loot::MessageType::say)
      node["type"] = "say";
    else if (rhs.GetType() == loot::MessageType::warn)
      node["type"] = "warn";
    else
      node["type"] = "error";

    if (rhs.IsConditional())
      node["condition"] = rhs.GetCondition();

    return node;
  }

  static bool decode(const Node& node, loot::Message& rhs) {
    if (!node.IsMap())
      throw RepresentationException(node.Mark(), "bad conversion: 'message' object must be a map");
    if (!node["type"])
      throw RepresentationException(node.Mark(), "bad conversion: 'type' key missing from 'message' object");
    if (!node["content"])
      throw RepresentationException(node.Mark(), "bad conversion: 'content' key missing from 'message' object");

    std::string type;
    type = node["type"].as<std::string>();

    loot::MessageType typeNo = loot::MessageType::say;
    if (boost::iequals(type, "warn"))
      typeNo = loot::MessageType::warn;
    else if (boost::iequals(type, "error"))
      typeNo = loot::MessageType::error;

    std::vector<loot::MessageContent> content;
    if (node["content"].IsSequence())
      content = node["content"].as< std::vector<loot::MessageContent> >();
    else {
      content.push_back(loot::MessageContent(node["content"].as<std::string>()));
    }

    //Check now that at least one item in content is English if there are multiple items.
    if (content.size() > 1) {
      bool found = false;
      for (const auto &mc : content) {
        if (mc.GetLanguage() == loot::MessageContent::defaultLanguage)
          found = true;
      }
      if (!found)
        throw RepresentationException(node.Mark(), "bad conversion: multilingual messages must contain an English content string");
    }

    // Make any substitutions at this point.
    if (node["subs"]) {
      std::vector<std::string> subs = node["subs"].as<std::vector<std::string>>();
      for (auto& mc : content) {
        boost::format f(mc.GetText());

        for (const auto& sub : subs) {
          f = f % sub;
        }

        try {
          mc = loot::MessageContent(f.str(), mc.GetLanguage());
        } catch (boost::io::format_error& e) {
          throw RepresentationException(node.Mark(), std::string("bad conversion: content substitution error: ") + e.what());
        }
      }
    }

    std::string condition;
    if (node["condition"])
      condition = node["condition"].as<std::string>();

    rhs = loot::Message(typeNo, content, condition);

    // Test condition syntax.
    try {
      rhs.ParseCondition();
    } catch (std::exception& e) {
      throw RepresentationException(node.Mark(), std::string("bad conversion: invalid condition syntax: ") + e.what());
    }

    return true;
  }
};

inline Emitter& operator << (Emitter& out, const loot::Message& rhs) {
  out << BeginMap;

  if (rhs.GetType() == loot::MessageType::say)
    out << Key << "type" << Value << "say";
  else if (rhs.GetType() == loot::MessageType::warn)
    out << Key << "type" << Value << "warn";
  else
    out << Key << "type" << Value << "error";

  if (rhs.GetContent().size() == 1)
    out << Key << "content" << Value << YAML::SingleQuoted << rhs.GetContent().front().GetText();
  else
    out << Key << "content" << Value << rhs.GetContent();

  if (rhs.IsConditional())
    out << Key << "condition" << Value << YAML::SingleQuoted << rhs.GetCondition();

  out << EndMap;

  return out;
}
}

#endif
