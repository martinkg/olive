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

#ifndef TRACKLISTOUTPUT_H
#define TRACKLISTOUTPUT_H

#include "common/timelinecommon.h"
#include "node/block/block.h"
#include "node/output/track/track.h"

/**
 * @brief Node that represents a collection of Tracks of a certain type
 */
class TrackListOutput : public Node
{
  Q_OBJECT
public:
  enum TrackType {
    kNone = -1,
    kVideo,
    kAudio,
    kSubtitle
  };

  TrackListOutput();

  virtual QString Name() override;
  virtual QString id() override;
  virtual QString Category() override;
  virtual QString Description() override;

  const rational& Timebase();
  void SetTimebase(const rational& timebase);

  NodeInput* track_input();

  const QVector<TrackOutput*>& Tracks();

  TrackOutput* TrackAt(int index);

  void AddTrack();

  void RemoveTrack();

  rational TimelineLength();

public slots:
  /**
   * @brief Slot for when the track connection is added
   */
  void TrackConnectionAdded(NodeEdgePtr edge);

  /**
   * @brief Slot for when the track connection is removed
   */
  void TrackConnectionRemoved(NodeEdgePtr edge);

  /**
   * @brief Slot for when a connected Track has added a Block so we can update the UI
   */
  void TrackAddedBlock(Block* block);

  /**
   * @brief Slot for when a connected Track has added a Block so we can update the UI
   */
  void TrackRemovedBlock(Block* block);

  /**
   * @brief Slot for when an attached Track has an edge added
   */
  void TrackEdgeAdded(NodeEdgePtr edge);

  /**
   * @brief Slot for when an attached Track has an edge added
   */
  void TrackEdgeRemoved(NodeEdgePtr edge);

signals:
  void TimebaseChanged(const rational &timebase);

  void TimelineCleared();
  
  void BlockAdded(Block* block, int index);
  
  void BlockRemoved(Block* block);

  void TrackAdded(TrackOutput* track);

  void TrackRemoved(TrackOutput* track);

protected:

private:
  TrackOutput* attached_track();

  void AttachTrack(TrackOutput *track);

  void DetachTrack(TrackOutput* track);

  static TrackOutput* TrackFromBlock(Block* block);

  NodeInput* track_input_;

  /**
   * @brief A cache of connected Tracks
   */
  QVector<TrackOutput*> track_cache_;

  rational timebase_;

};

#endif // TRACKLISTOUTPUT_H
