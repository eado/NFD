/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2017,  Regents of the University of California,
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

#include "face/ethernet-factory.hpp"
#include "face/face.hpp"

#include "ethernet-fixture.hpp"
#include "face-system-fixture.hpp"
#include "factory-test-common.hpp"
#include <boost/algorithm/string/replace.hpp>
#include <boost/range/algorithm/count_if.hpp>

namespace nfd {
namespace face {
namespace tests {

class EthernetFactoryFixture : public EthernetFixture
                             , public FaceSystemFactoryFixture<EthernetFactory>
{
protected:
  EthernetFactoryFixture()
  {
    this->copyRealNetifsToNetmon();
  }

  std::vector<const Face*>
  listEtherMcastFaces(ndn::nfd::LinkType linkType = ndn::nfd::LINK_TYPE_MULTI_ACCESS) const
  {
    return this->listFacesByScheme("ether", linkType);
  }

  size_t
  countEtherMcastFaces(ndn::nfd::LinkType linkType = ndn::nfd::LINK_TYPE_MULTI_ACCESS) const
  {
    return this->listEtherMcastFaces(linkType).size();
  }
};

BOOST_AUTO_TEST_SUITE(Face)
BOOST_FIXTURE_TEST_SUITE(TestEthernetFactory, EthernetFactoryFixture)

using nfd::Face;

BOOST_AUTO_TEST_SUITE(ProcessConfig)

BOOST_AUTO_TEST_CASE(Normal)
{
  SKIP_IF_ETHERNET_NETIF_COUNT_LT(1);

  const std::string CONFIG = R"CONFIG(
    face_system
    {
      ether
      {
        mcast yes
        mcast_group 01:00:5E:00:17:AA
        mcast_ad_hoc no
        whitelist
        {
          *
        }
        blacklist
        {
        }
      }
    }
  )CONFIG";

  parseConfig(CONFIG, true);
  parseConfig(CONFIG, false);

  BOOST_CHECK_EQUAL(this->countEtherMcastFaces(), netifs.size());
}

BOOST_AUTO_TEST_CASE(EnableDisableMcast)
{
  const std::string CONFIG_WITH_MCAST = R"CONFIG(
    face_system
    {
      ether
      {
        mcast yes
      }
    }
  )CONFIG";
  const std::string CONFIG_WITHOUT_MCAST = R"CONFIG(
    face_system
    {
      ether
      {
        mcast no
      }
    }
  )CONFIG";

  parseConfig(CONFIG_WITHOUT_MCAST, false);
  BOOST_CHECK_EQUAL(this->countEtherMcastFaces(), 0);

  SKIP_IF_ETHERNET_NETIF_COUNT_LT(1);

  parseConfig(CONFIG_WITH_MCAST, false);
  g_io.poll();
  BOOST_CHECK_EQUAL(this->countEtherMcastFaces(), netifs.size());

  parseConfig(CONFIG_WITHOUT_MCAST, false);
  g_io.poll();
  BOOST_CHECK_EQUAL(this->countEtherMcastFaces(), 0);
}

BOOST_AUTO_TEST_CASE(McastAdHoc)
{
  SKIP_IF_ETHERNET_NETIF_COUNT_LT(1);

  const std::string CONFIG = R"CONFIG(
    face_system
    {
      ether
      {
        mcast_ad_hoc yes
      }
    }
  )CONFIG";

  parseConfig(CONFIG, false);
  BOOST_CHECK_EQUAL(this->countEtherMcastFaces(ndn::nfd::LINK_TYPE_AD_HOC), netifs.size());
}

BOOST_AUTO_TEST_CASE(ChangeMcastGroup)
{
  SKIP_IF_ETHERNET_NETIF_COUNT_LT(1);

  const std::string CONFIG1 = R"CONFIG(
    face_system
    {
      ether
      {
        mcast_group 01:00:5e:90:10:01
      }
    }
  )CONFIG";
  const std::string CONFIG2 = R"CONFIG(
    face_system
    {
      ether
      {
        mcast_group 01:00:5e:90:10:02
      }
    }
  )CONFIG";

  parseConfig(CONFIG1, false);
  auto etherMcastFaces = this->listEtherMcastFaces();
  BOOST_REQUIRE_EQUAL(etherMcastFaces.size(), netifs.size());
  BOOST_CHECK_EQUAL(etherMcastFaces.front()->getRemoteUri(),
                    FaceUri(ethernet::Address{0x01, 0x00, 0x5e, 0x90, 0x10, 0x01}));

  parseConfig(CONFIG2, false);
  g_io.poll();
  etherMcastFaces = this->listEtherMcastFaces();
  BOOST_REQUIRE_EQUAL(etherMcastFaces.size(), netifs.size());
  BOOST_CHECK_EQUAL(etherMcastFaces.front()->getRemoteUri(),
                    FaceUri(ethernet::Address{0x01, 0x00, 0x5e, 0x90, 0x10, 0x02}));
}

BOOST_AUTO_TEST_CASE(Whitelist)
{
  SKIP_IF_ETHERNET_NETIF_COUNT_LT(1);

  std::string CONFIG = R"CONFIG(
    face_system
    {
      ether
      {
        whitelist
        {
          ifname %ifname
        }
      }
    }
  )CONFIG";
  boost::replace_first(CONFIG, "%ifname", netifs.front()->getName());

  parseConfig(CONFIG, false);
  auto etherMcastFaces = this->listEtherMcastFaces();
  BOOST_REQUIRE_EQUAL(etherMcastFaces.size(), 1);
  BOOST_CHECK_EQUAL(etherMcastFaces.front()->getLocalUri().getHost(), netifs.front()->getName());
}

BOOST_AUTO_TEST_CASE(Blacklist)
{
  SKIP_IF_ETHERNET_NETIF_COUNT_LT(1);

  std::string CONFIG = R"CONFIG(
    face_system
    {
      ether
      {
        blacklist
        {
          ifname %ifname
        }
      }
    }
  )CONFIG";
  boost::replace_first(CONFIG, "%ifname", netifs.front()->getName());

  parseConfig(CONFIG, false);
  auto etherMcastFaces = this->listEtherMcastFaces();
  BOOST_CHECK_EQUAL(etherMcastFaces.size(), netifs.size() - 1);
  BOOST_CHECK_EQUAL(boost::count_if(etherMcastFaces, [=] (const Face* face) {
    return face->getLocalUri().getHost() == netifs.front()->getName();
  }), 0);
}

BOOST_AUTO_TEST_CASE(Omitted)
{
  const std::string CONFIG = R"CONFIG(
    face_system
    {
    }
  )CONFIG";

  parseConfig(CONFIG, true);
  parseConfig(CONFIG, false);

  BOOST_CHECK_EQUAL(this->countEtherMcastFaces(), 0);
}

BOOST_AUTO_TEST_CASE(BadMcast)
{
  const std::string CONFIG = R"CONFIG(
    face_system
    {
      ether
      {
        mcast hello
      }
    }
  )CONFIG";

  BOOST_CHECK_THROW(parseConfig(CONFIG, true), ConfigFile::Error);
  BOOST_CHECK_THROW(parseConfig(CONFIG, false), ConfigFile::Error);
}

BOOST_AUTO_TEST_CASE(BadMcastGroup)
{
  const std::string CONFIG = R"CONFIG(
    face_system
    {
      ether
      {
        mcast yes
        mcast_group hello
      }
    }
  )CONFIG";

  BOOST_CHECK_THROW(parseConfig(CONFIG, true), ConfigFile::Error);
  BOOST_CHECK_THROW(parseConfig(CONFIG, false), ConfigFile::Error);
}

BOOST_AUTO_TEST_CASE(UnicastMcastGroup)
{
  const std::string CONFIG = R"CONFIG(
    face_system
    {
      ether
      {
        mcast yes
        mcast_group 00:00:5e:00:53:5e
      }
    }
  )CONFIG";

  BOOST_CHECK_THROW(parseConfig(CONFIG, true), ConfigFile::Error);
  BOOST_CHECK_THROW(parseConfig(CONFIG, false), ConfigFile::Error);
}

BOOST_AUTO_TEST_CASE(UnknownOption)
{
  const std::string CONFIG = R"CONFIG(
    face_system
    {
      ether
      {
        hello
      }
    }
  )CONFIG";

  BOOST_CHECK_THROW(parseConfig(CONFIG, true), ConfigFile::Error);
  BOOST_CHECK_THROW(parseConfig(CONFIG, false), ConfigFile::Error);
}

BOOST_AUTO_TEST_SUITE_END() // ProcessConfig

BOOST_AUTO_TEST_CASE(GetChannels)
{
  auto channels = factory.getChannels();
  BOOST_CHECK_EQUAL(channels.empty(), true);
}

BOOST_AUTO_TEST_CASE(UnsupportedFaceCreate)
{
  createFace(factory,
             FaceUri("ether://[00:00:5e:00:53:5e]"),
             {},
             ndn::nfd::FACE_PERSISTENCY_PERSISTENT,
             false,
             {CreateFaceExpectedResult::FAILURE, 406,
              "Creation of unicast Ethernet faces requires a LocalUri with dev:// scheme"});

  createFace(factory,
             FaceUri("ether://[00:00:5e:00:53:5e]"),
             FaceUri("udp4://127.0.0.1:20071"),
             ndn::nfd::FACE_PERSISTENCY_PERSISTENT,
             false,
             {CreateFaceExpectedResult::FAILURE, 406,
              "Creation of unicast Ethernet faces requires a LocalUri with dev:// scheme"});

  createFace(factory,
             FaceUri("ether://[00:00:5e:00:53:5e]"),
             FaceUri("dev://eth0"),
             ndn::nfd::FACE_PERSISTENCY_ON_DEMAND,
             false,
             {CreateFaceExpectedResult::FAILURE, 406,
              "Outgoing Ethernet faces do not support on-demand persistency"});

  createFace(factory,
             FaceUri("ether://[01:00:5e:90:10:5e]"),
             FaceUri("dev://eth0"),
             ndn::nfd::FACE_PERSISTENCY_PERSISTENT,
             false,
             {CreateFaceExpectedResult::FAILURE, 406,
              "Cannot create multicast Ethernet faces"});

  createFace(factory,
             FaceUri("ether://[00:00:5e:00:53:5e]"),
             FaceUri("dev://eth0"),
             ndn::nfd::FACE_PERSISTENCY_PERSISTENT,
             true,
             {CreateFaceExpectedResult::FAILURE, 406,
              "Local fields can only be enabled on faces with local scope"});
}

BOOST_AUTO_TEST_SUITE_END() // TestEthernetFactory
BOOST_AUTO_TEST_SUITE_END() // Face

} // namespace tests
} // namespace face
} // namespace nfd
