#pragma once
#include "multiplayerinterface.h"
#include <enet/enet.h>
#include <unordered_map>

namespace FAWorld
{
    class World;
}

namespace Engine
{
    class LocalInputHandler;

    class Server : public MultiplayerInterface
    {
    public:
        Server(FAWorld::World& world, LocalInputHandler& localInputHandler);
        virtual ~Server();

        virtual boost::optional<std::vector<FAWorld::PlayerInput>> getAndClearInputs(FAWorld::Tick tick) override;
        virtual void update() override;
        virtual void verify(FAWorld::Tick) override {}
        virtual bool isServer() const override { return true; }

    private:
        struct Peer
        {
            Peer() = default;
            explicit Peer(ENetPeer* peer) : peer(peer) {}

            ENetPeer* peer = nullptr;
            bool hasMap = false;
            FAWorld::Tick lastTick = 0;
        };

        void onPeerConnect(const ENetEvent& event);
        void readPeerPacket(const ENetEvent& event);
        void sendInputsToClients(std::vector<FAWorld::PlayerInput>& inputs);
        void receiveClientUpdate(FASaveGame::GameLoader& loader, Peer& peer);

        bool mDoFullVerify = false;

        FAWorld::World& mWorld;
        LocalInputHandler& mLocalInputHandler;

        // This is where we accumulate inputs received from clients before we execute them
        std::vector<FAWorld::PlayerInput> mInputsBuffer;
        // This is where we locally store inputs to be executed by the server.
        // They are accumulated in mInputsBuffer, and eventually both sent to clients and moved into here.
        std::unordered_map<FAWorld::Tick, std::vector<FAWorld::PlayerInput>> mInputs;

        FAWorld::Tick mLastSentTick = -1;

        ENetHost* mHost = nullptr;
        ENetAddress mAddress;
        std::map<uint32_t, Peer> mPeers;
    };
}
