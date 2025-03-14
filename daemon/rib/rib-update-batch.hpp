/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2025,  Regents of the University of California,
 *                           Arizona Board of Regents,
 *                           Colorado State University,
 *                           University Pierre & Marie Curie, Sorbonne University,
 *                           Washington University in St. Louis,
 *                           Beijing Institute of Technology,
 *                           The University of Memphis.
 *
 * This file is part of NFD (Named Data Networking Forwarding Daemon).
 * See AUTHORS.md for complete list of NFD authors and contributors.
 *
 * NFD is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * NFD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * NFD, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef NFD_DAEMON_RIB_RIB_UPDATE_BATCH_HPP
#define NFD_DAEMON_RIB_RIB_UPDATE_BATCH_HPP

#include "route.hpp"

#include <list>

namespace nfd::rib {

/**
 * \brief Represents a route that will be added to or removed from a namespace.
 */
struct RibUpdate
{
  enum Action {
    REGISTER    = 0,
    UNREGISTER  = 1,
    /**
     * \brief An update triggered by a face destruction notification
     * \note indicates a Route needs to be removed after a face is destroyed
     */
    REMOVE_FACE = 2,
  };

  Action action;
  Name name;
  Route route;
};

std::ostream&
operator<<(std::ostream& os, RibUpdate::Action action);

std::ostream&
operator<<(std::ostream& os, const RibUpdate& update);

using RibUpdateList = std::list<RibUpdate>;

/**
 * \brief Represents a collection of RibUpdates to be applied to a single FaceId.
 */
class RibUpdateBatch
{
public:
  using const_iterator = RibUpdateList::const_iterator;

  explicit
  RibUpdateBatch(uint64_t faceId);

  uint64_t
  getFaceId() const noexcept
  {
    return m_faceId;
  }

  void
  add(const RibUpdate& update);

  const_iterator
  begin() const noexcept
  {
    return m_updates.begin();
  }

  const_iterator
  end() const noexcept
  {
    return m_updates.end();
  }

  size_t
  size() const noexcept
  {
    return m_updates.size();
  }

private:
  uint64_t m_faceId;
  RibUpdateList m_updates;
};

} // namespace nfd::rib

#endif // NFD_DAEMON_RIB_RIB_UPDATE_BATCH_HPP
