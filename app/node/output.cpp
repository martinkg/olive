/***

  Olive - Non-Linear Video Editor
  Copyright (C) 2019 Olive Team

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

***/

#include "output.h"

#include "node/node.h"

NodeOutput::NodeOutput(const QString &id) :
  NodeParam(id)
{
}

NodeParam::Type NodeOutput::type()
{
  return kOutput;
}

QVariant NodeOutput::get_realtime_value()
{
  QVariant v = parent()->Value(this);

  return v;
}

bool NodeOutput::has_cached_value(const TimeRange &time)
{
  return cached_values_.contains(time);
}

QVariant NodeOutput::get_cached_value(const TimeRange &time)
{
  return cached_values_.value(time);
}

void NodeOutput::cache_value(const TimeRange &time, const QVariant &value)
{
  cached_values_.insert(time, value);
}

void NodeOutput::drop_cached_values()
{
  cached_values_.clear();
}
