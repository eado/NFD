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

#ifndef NFD_DAEMON_MGMT_FIB_MANAGER_HPP
#define NFD_DAEMON_MGMT_FIB_MANAGER_HPP

#include "manager-base.hpp"

namespace nfd {

namespace fib {
class Fib;
} // namespace fib

class FaceTable;

/**
 * @brief Implements the FIB Management of NFD Management Protocol.
 * @sa https://redmine.named-data.net/projects/nfd/wiki/FibMgmt
 */
class FibManager final : public ManagerBase
{
public:
  FibManager(fib::Fib& fib, const FaceTable& faceTable,
             Dispatcher& dispatcher, CommandAuthenticator& authenticator);

private:
  void
  addNextHop(const Interest& interest, ControlParameters parameters,
             const CommandContinuation& done);

  void
  removeNextHop(const Interest& interest, ControlParameters parameters,
                const CommandContinuation& done);

  void
  listEntries(ndn::mgmt::StatusDatasetContext& context);

private:
  void
  setFaceForSelfRegistration(const Interest& request, ControlParameters& parameters);

private:
  fib::Fib& m_fib;
  const FaceTable& m_faceTable;
};

} // namespace nfd

#endif // NFD_DAEMON_MGMT_FIB_MANAGER_HPP
