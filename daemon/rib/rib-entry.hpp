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

#ifndef NFD_DAEMON_RIB_RIB_ENTRY_HPP
#define NFD_DAEMON_RIB_RIB_ENTRY_HPP

#include "route.hpp"

#include <list>

namespace nfd::rib {

/**
 * \brief Represents a RIB entry, which contains one or more Routes with the same prefix.
 */
class RibEntry : public std::enable_shared_from_this<RibEntry>
{
public:
  using RouteList = std::list<Route>;
  using iterator = RouteList::iterator;
  using const_iterator = RouteList::const_iterator;

  const Name&
  getName() const noexcept
  {
    return m_name;
  }

  void
  setName(const Name& prefix)
  {
    m_name = prefix;
  }

  shared_ptr<RibEntry>
  getParent() const
  {
    return m_parent;
  }

  const std::list<shared_ptr<RibEntry>>&
  getChildren() const noexcept
  {
    return m_children;
  }

  void
  addChild(shared_ptr<RibEntry> child);

  void
  removeChild(shared_ptr<RibEntry> child);

  /** \brief Inserts a new route into the entry's route list.
   *
   *  If another route already exists with the same faceId and origin,
   *  the new route is not inserted.
   *
   *  \return A pair, whose first element is the iterator to the newly
   *  inserted element if the insert succeeds and to the
   *  previously-existing element otherwise, and whose second element
   *  is true if the insert succeeds and false otherwise.
   */
  std::pair<RibEntry::iterator, bool>
  insertRoute(const Route& route);

  /**
   * \brief Erases a Route with the same FaceId and origin.
   */
  void
  eraseRoute(const Route& route);

  /**
   * \brief Erases a Route with the passed iterator.
   * \return An iterator to the element that followed the erased iterator
   */
  iterator
  eraseRoute(RouteList::iterator route);

  bool
  hasFaceId(uint64_t faceId) const;

  iterator
  findRoute(const Route& route);

  const_iterator
  findRoute(const Route& route) const;

  void
  addInheritedRoute(const Route& route);

  void
  removeInheritedRoute(const Route& route);

  /**
   * \brief Returns the routes this namespace has inherited.
   *
   * The inherited routes returned represent inherited routes this namespace has in the FIB.
   */
  const RouteList&
  getInheritedRoutes() const noexcept
  {
    return m_inheritedRoutes;
  }

  /**
   * \brief Finds an inherited route with a matching face ID.
   * \return An iterator to the matching route if one is found;
   *         otherwise, an iterator to the end of the entry's
   *         inherited route list
   */
  RouteList::const_iterator
  findInheritedRoute(const Route& route) const;

  /** \brief Determines if the entry has an inherited route with a matching face ID.
   *  \return True, if a matching inherited route is found; otherwise, false.
   */
  bool
  hasInheritedRoute(const Route& route) const;

  bool
  hasCapture() const noexcept
  {
    return m_nRoutesWithCaptureSet > 0;
  }

  /** \brief Determines if the entry has an inherited route with the passed
   *         face ID and its child inherit flag set.
   *  \return True, if a matching inherited route is found; otherwise, false.
   */
  bool
  hasChildInheritOnFaceId(uint64_t faceId) const;

  /** \brief Returns the route with the lowest cost that has the passed face ID.
   *  \return The route with the lowest cost that has the passed face ID
   */
  const Route*
  getRouteWithLowestCostByFaceId(uint64_t faceId) const;

  const Route*
  getRouteWithSecondLowestCostByFaceId(uint64_t faceId) const;

  /** \brief Returns the route with the lowest cost that has the passed face ID
   *         and its child inherit flag set.
   */
  const Route*
  getRouteWithLowestCostAndChildInheritByFaceId(uint64_t faceId) const;

  /** \brief Retrieve a prefix announcement suitable for readvertising this route.
   *
   *  If one or more routes in this RIB entry contains a prefix announcement, this method returns
   *  the announcement from the route that expires last.
   *
   *  If this RIB entry does not have a route containing a prefix announcement, this method creates
   *  a new announcement. Its expiration period reflects the remaining lifetime of this RIB entry,
   *  confined within [\p minExpiration, \p maxExpiration] range. The caller is expected to sign
   *  this announcement.
   *
   *  \warning `minExpiration > maxExpiration` triggers undefined behavior.
   */
  ndn::PrefixAnnouncement
  getPrefixAnnouncement(time::milliseconds minExpiration = 15_s,
                        time::milliseconds maxExpiration = 1_h) const;

  const RouteList&
  getRoutes() const noexcept
  {
    return m_routes;
  }

  bool
  empty() const noexcept
  {
    return m_routes.empty();
  }

  const_iterator
  begin() const noexcept
  {
    return m_routes.begin();
  }

  const_iterator
  end() const noexcept
  {
    return m_routes.end();
  }

  iterator
  begin() noexcept
  {
    return m_routes.begin();
  }

  iterator
  end() noexcept
  {
    return m_routes.end();
  }

private:
  Name m_name;
  std::list<shared_ptr<RibEntry>> m_children;
  shared_ptr<RibEntry> m_parent;
  RouteList m_routes;
  RouteList m_inheritedRoutes;

  /** \brief The number of routes on this namespace with the capture flag set.
   *
   *  This count is used to check if the namespace will block inherited routes.
   *  If the number is greater than zero, a route on the namespace has its capture
   *  flag set which means the namespace should not inherit any routes.
   */
  uint64_t m_nRoutesWithCaptureSet = 0;
};

std::ostream&
operator<<(std::ostream& os, const RibEntry& entry);

} // namespace nfd::rib

#endif // NFD_DAEMON_RIB_RIB_ENTRY_HPP
