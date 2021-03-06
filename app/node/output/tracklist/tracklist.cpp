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

#include "tracklist.h"

#include <QDebug>

#include "node/blend/alphaover/alphaover.h"
#include "node/block/gap/gap.h"
#include "node/graph.h"
#include "panel/timeline/timeline.h"

TrackListOutput::TrackListOutput()
{
  track_input_ = new NodeInput("track_in");
  track_input_->add_data_input(NodeParam::kTrack);
  AddParameter(track_input_);

  connect(this, SIGNAL(EdgeAdded(NodeEdgePtr)), this, SLOT(TrackConnectionAdded(NodeEdgePtr)));
  connect(this, SIGNAL(EdgeRemoved(NodeEdgePtr)), this, SLOT(TrackConnectionRemoved(NodeEdgePtr)));
}

QString TrackListOutput::Name()
{
  return tr("Track List");
}

QString TrackListOutput::id()
{
  return "org.olivevideoeditor.Olive.tracklist";
}

QString TrackListOutput::Category()
{
  return tr("Output");
}

QString TrackListOutput::Description()
{
  return tr("Node for collecting Tracks of the same type.");
}

const rational &TrackListOutput::Timebase()
{
  return timebase_;
}

NodeInput *TrackListOutput::track_input()
{
  return track_input_;
}

const QVector<TrackOutput *> &TrackListOutput::Tracks()
{
  return track_cache_;
}

TrackOutput *TrackListOutput::TrackAt(int index)
{
  return track_cache_.at(index);
}

rational TrackListOutput::TimelineLength()
{
  rational length = 0;

  foreach (TrackOutput* track, track_cache_) {
    length = qMax(length, track->in());
  }

  return length;
}

TrackOutput *TrackListOutput::attached_track()
{
  return ValueToPtr<TrackOutput>(track_input_->get_value(0));
}

void TrackListOutput::AttachTrack(TrackOutput *track)
{
  TrackOutput* current_track = track;

  // Traverse through Tracks caching and connecting them
  while (current_track != nullptr) {
    connect(current_track, SIGNAL(EdgeAdded(NodeEdgePtr)), this, SLOT(TrackEdgeAdded(NodeEdgePtr)));
    connect(current_track, SIGNAL(EdgeRemoved(NodeEdgePtr)), this, SLOT(TrackEdgeRemoved(NodeEdgePtr)));
    connect(current_track, SIGNAL(BlockAdded(Block*)), this, SLOT(TrackAddedBlock(Block*)));
    connect(current_track, SIGNAL(BlockRemoved(Block*)), this, SLOT(TrackRemovedBlock(Block*)));

    current_track->SetIndex(track_cache_.size());

    track_cache_.append(current_track);

    // This function must be called after the track is added to track_cache_, since it uses track_cache_ to determine
    // the track's index
    emit TrackAdded(current_track);

    current_track = current_track->next_track();
  }
}

void TrackListOutput::DetachTrack(TrackOutput *track)
{
  TrackOutput* current_track = track;

  // Traverse through Tracks uncaching and disconnecting them
  while (current_track != nullptr) {
    emit TrackRemoved(current_track);

    current_track->SetIndex(-1);

    disconnect(current_track, SIGNAL(EdgeAdded(NodeEdgePtr)), this, SLOT(TrackEdgeAdded(NodeEdgePtr)));
    disconnect(current_track, SIGNAL(EdgeRemoved(NodeEdgePtr)), this, SLOT(TrackEdgeRemoved(NodeEdgePtr)));
    disconnect(current_track, SIGNAL(BlockAdded(Block*)), this, SLOT(TrackAddedBlock(Block*)));
    disconnect(current_track, SIGNAL(BlockRemoved(Block*)), this, SLOT(TrackRemovedBlock(Block*)));

    track_cache_.removeAll(current_track);

    current_track = current_track->next_track();
  }
}

void TrackListOutput::SetTimebase(const rational &timebase)
{
  timebase_ = timebase;

  emit TimebaseChanged(timebase_);
}

void TrackListOutput::AddTrack()
{
  TrackOutput* track = new TrackOutput();
  static_cast<NodeGraph*>(parent())->AddNode(track);

  if (track_cache_.isEmpty()) {
    // Connect this track directly to this output
    NodeParam::ConnectEdge(track->track_output(), track_input());
  } else {
    TrackOutput* current_last_track = track_cache_.last();

    // Connect this track to the current last track
    NodeParam::ConnectEdge(track->track_output(), current_last_track->track_input());

    // FIXME: Test code only
    AlphaOverBlend* blend = new AlphaOverBlend();
    static_cast<NodeGraph*>(parent())->AddNode(blend);

    NodeParam::ConnectEdge(track->texture_output(), blend->blend_input());
    NodeParam::ConnectEdge(current_last_track->texture_output(), blend->base_input());
    NodeParam::ConnectEdge(blend->texture_output(), current_last_track->texture_output()->edges().first()->input());
    // End test code
  }
}

void TrackListOutput::RemoveTrack()
{
  if (track_cache_.isEmpty()) {
    return;
  }

  TrackOutput* track = track_cache_.last();

  static_cast<NodeGraph*>(parent())->TakeNode(track);

  delete track;
}

TrackOutput *TrackListOutput::TrackFromBlock(Block *block)
{
  Block* n = block;

  // Find last valid block in Sequence and assume its a track
  while (n->next() != nullptr) {
    n = n->next();
  }

  // Downside of this approach is the usage of dynamic_cast, alternative would be looping through all known tracks and
  // seeing if the contain the Block, but this seems slower
  return dynamic_cast<TrackOutput*>(n);
}

void TrackListOutput::TrackConnectionAdded(NodeEdgePtr edge)
{
  if (edge->input() != track_input()) {
    return;
  }

  AttachTrack(attached_track());

  // FIXME: Is this necessary?
  emit TimebaseChanged(timebase_);
}

void TrackListOutput::TrackConnectionRemoved(NodeEdgePtr edge)
{
  if (edge->input() != track_input()) {
    return;
  }

  DetachTrack(ValueToPtr<TrackOutput>(edge->output()->get_value(0)));

  emit TimelineCleared();
}

void TrackListOutput::TrackAddedBlock(Block *block)
{
  emit BlockAdded(block, static_cast<TrackOutput*>(sender())->Index());
}

void TrackListOutput::TrackRemovedBlock(Block *block)
{
  emit BlockRemoved(block);
}

void TrackListOutput::TrackEdgeAdded(NodeEdgePtr edge)
{
  // Assume this signal was sent from a TrackOutput
  TrackOutput* track = static_cast<TrackOutput*>(sender());

  // If this edge pertains to the track's track input, all the tracks just added need attaching
  if (edge->input() == track->track_input()) {
    TrackOutput* added_track = ValueToPtr<TrackOutput>(edge->output()->get_value(0));

    AttachTrack(added_track);
  }
}

void TrackListOutput::TrackEdgeRemoved(NodeEdgePtr edge)
{
  // Assume this signal was sent from a TrackOutput
  TrackOutput* track = static_cast<TrackOutput*>(sender());

  // If this edge pertains to the track's track input, all the tracks just added need attaching
  if (edge->input() == track->track_input()) {
    TrackOutput* added_track = ValueToPtr<TrackOutput>(edge->output()->get_value(0));

    DetachTrack(added_track);
  }
}
