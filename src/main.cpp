#include <gtest/gtest.h>
#include <mpsc_ring.hpp>
#include <thread>

using mpsc_ring::Sender;
using mpsc_ring::Receiver;
using mpsc_ring::channel;
using mpsc_ring::TryRecvError;

template <typename T>
void repetitive_send(Sender<T> sender, T x, size_t n) {
	while (n) {
		n -= 1;
		sender.send(x);
	}
}
template <typename T>
void repetitive_recv(Receiver<T> receiver, T expected, size_t n) {
	while (n) {
		n -= 1;
		auto ret = receiver.recv();
		ASSERT_TRUE(ret.has_value());
		ASSERT_EQ(ret.value(), expected);
	}
	ASSERT_EQ(receiver.recv(), std::nullopt);
}

template <typename T>
void spsc_repetitive_send_recv(size_t size, T x, size_t n) {
	auto [sender, receiver] = channel<T>(size);
	std::thread s(repetitive_send<T>, std::move(sender), x, n);
	std::thread r(repetitive_recv<T>, std::move(receiver), x, n);
	s.join();
	r.join();
}

TEST(SPSC, RepetitiveSendRecvIntSize1Num1) {
	spsc_repetitive_send_recv(1, 233, 1);
}

TEST(SPSC, RepetitiveSendRecvIntSize1Num1e5) {
	spsc_repetitive_send_recv(1, 233, 100000);
}
TEST(SPSC, RepetitiveSendRecvIntSize2Num1e5) {
	spsc_repetitive_send_recv(2, 233, 100000);
}
TEST(SPSC, RepetitiveSendRecvIntSize4Num1e5) {
	spsc_repetitive_send_recv(4, 233, 100000);
}
TEST(SPSC, RepetitiveSendRecvIntSize8Num1e5) {
	spsc_repetitive_send_recv(8, 233, 100000);
}
TEST(SPSC, RepetitiveSendRecvIntSize16Num1e5) {
	spsc_repetitive_send_recv(16, 233, 100000);
}
TEST(SPSC, RepetitiveSendRecvIntSize32Num1e5) {
	spsc_repetitive_send_recv(32, 233, 100000);
}

TEST(SPSC, RepetitiveSendRecvIntSize8Num1e6) {
	spsc_repetitive_send_recv(8, 233, 1000000);
}
TEST(SPSC, RepetitiveSendRecvIntSize16Num1e6) {
	spsc_repetitive_send_recv(16, 233, 1000000);
}
TEST(SPSC, RepetitiveSendRecvIntSize32Num1e6) {
	spsc_repetitive_send_recv(32, 233, 1000000);
}
TEST(SPSC, RepetitiveSendRecvIntSize64Num1e6) {
	spsc_repetitive_send_recv(64, 233, 1000000);
}
TEST(SPSC, RepetitiveSendRecvIntSize128Num1e6) {
	spsc_repetitive_send_recv(128, 233, 1000000);
}

template <typename T>
void spsc_repetitive_try_recv_send_try_recv(size_t size, T x, size_t n) {
	auto [sender, receiver] = channel<T>(size);
	while (n) {
		n -= 1;
		ASSERT_EQ(receiver.try_recv().unwrap_err(), TryRecvError::Empty);
		sender.send(x);
		ASSERT_EQ(receiver.try_recv().unwrap(), x);
	}
	ASSERT_EQ(receiver.try_recv().unwrap_err(), TryRecvError::Empty);
	sender.drop();
	ASSERT_EQ(receiver.try_recv().unwrap_err(), TryRecvError::Disconnected);
}

TEST(SPSC, RepetitiveTryRecvSendTryRecvIntSize1Num1) {
	spsc_repetitive_try_recv_send_try_recv(1, 233, 1);
}
TEST(SPSC, RepetitiveTryRecvSendTryRecvIntSize1Num1e7) {
	spsc_repetitive_try_recv_send_try_recv(1, 233, 10000000);
}
TEST(SPSC, RepetitiveTryRecvSendTryRecvIntSize2pow20Num1e7) {
	spsc_repetitive_try_recv_send_try_recv(1 << 20, 233, 10000000);
}

template <typename T>
void sp_sharded_consumers_repetitive_send_recv(
	size_t num_shards, size_t msg_per_shard, size_t size, T x
) {
	std::vector<std::thread> rs;
	{
		std::vector<Sender<T>> senders;
		for (size_t i = 0; i < num_shards; ++i) {
			auto [sender, receiver] = channel<T>(size);
			rs.emplace_back(
				repetitive_recv<T>, std::move(receiver), x, msg_per_shard
			);
			senders.emplace_back(std::move(sender));
		}
		for (size_t i = 0; i < msg_per_shard; ++i) {
			for (Sender<T> &sender : senders) {
				sender.send(x);
			}
		}
	}
	for (std::thread &r : rs) {
		r.join();
	}
}

TEST(SPShardedConsumers, RepetitiveSendRecvIntShard1MsgPerShard1e5Size1) {
	sp_sharded_consumers_repetitive_send_recv(1, 100000, 1, 233);
}
TEST(SPShardedConsumers, RepetitiveSendRecvIntShard1MsgPerShard1e5Size2) {
	sp_sharded_consumers_repetitive_send_recv(1, 100000, 2, 233);
}
TEST(SPShardedConsumers, RepetitiveSendRecvIntShard1MsgPerShard1e5Size4) {
	sp_sharded_consumers_repetitive_send_recv(1, 100000, 4, 233);
}
TEST(SPShardedConsumers, RepetitiveSendRecvIntShard1MsgPerShard1e5Size8) {
	sp_sharded_consumers_repetitive_send_recv(1, 100000, 8, 233);
}
TEST(SPShardedConsumers, RepetitiveSendRecvIntShard1MsgPerShard1e5Size16) {
	sp_sharded_consumers_repetitive_send_recv(1, 100000, 16, 233);
}
TEST(SPShardedConsumers, RepetitiveSendRecvIntShard1MsgPerShard1e5Size32) {
	sp_sharded_consumers_repetitive_send_recv(1, 100000, 32, 233);
}
TEST(SPShardedConsumers, RepetitiveSendRecvIntShard1MsgPerShard1e5Size64) {
	sp_sharded_consumers_repetitive_send_recv(1, 100000, 64, 233);
}

TEST(SPShardedConsumers, RepetitiveSendRecvIntShard2MsgPerShard1e5Size1) {
	sp_sharded_consumers_repetitive_send_recv(2, 100000, 1, 233);
}
TEST(SPShardedConsumers, RepetitiveSendRecvIntShard2MsgPerShard1e5Size2) {
	sp_sharded_consumers_repetitive_send_recv(2, 100000, 2, 233);
}
TEST(SPShardedConsumers, RepetitiveSendRecvIntShard2MsgPerShard1e5Size4) {
	sp_sharded_consumers_repetitive_send_recv(2, 100000, 4, 233);
}
TEST(SPShardedConsumers, RepetitiveSendRecvIntShard2MsgPerShard1e5Size8) {
	sp_sharded_consumers_repetitive_send_recv(2, 100000, 8, 233);
}
TEST(SPShardedConsumers, RepetitiveSendRecvIntShard2MsgPerShard1e5Size16) {
	sp_sharded_consumers_repetitive_send_recv(2, 100000, 16, 233);
}
TEST(SPShardedConsumers, RepetitiveSendRecvIntShard2MsgPerShard1e5Size32) {
	sp_sharded_consumers_repetitive_send_recv(2, 100000, 32, 233);
}
TEST(SPShardedConsumers, RepetitiveSendRecvIntShard2MsgPerShard1e5Size64) {
	sp_sharded_consumers_repetitive_send_recv(2, 100000, 64, 233);
}

TEST(SPShardedConsumers, RepetitiveSendRecvIntShard4MsgPerShard1e5Size1) {
	sp_sharded_consumers_repetitive_send_recv(4, 100000, 1, 233);
}
TEST(SPShardedConsumers, RepetitiveSendRecvIntShard4MsgPerShard1e5Size2) {
	sp_sharded_consumers_repetitive_send_recv(4, 100000, 2, 233);
}
TEST(SPShardedConsumers, RepetitiveSendRecvIntShard4MsgPerShard1e5Size4) {
	sp_sharded_consumers_repetitive_send_recv(4, 100000, 4, 233);
}
TEST(SPShardedConsumers, RepetitiveSendRecvIntShard4MsgPerShard1e5Size2pow20) {
	sp_sharded_consumers_repetitive_send_recv(4, 100000, 1 << 20, 233);
}

class CountConstructionDestruction {
public:
	CountConstructionDestruction(
		const CountConstructionDestruction &rhs
	) : c_(rhs.c_), d_(rhs.d_) {
		c_->fetch_add(1);
	}
	CountConstructionDestruction &operator=(
		const CountConstructionDestruction &rhs
	) {
		if (this == &rhs)
			return *this;
		this->~CountConstructionDestruction();
		c_ = rhs.c_;
		d_ = rhs.d_;
		c_->fetch_add(1);
		return *this;
	}
	CountConstructionDestruction(
		CountConstructionDestruction &&rhs
	) : c_(rhs.c_), d_(rhs.d_) {
		rhs.c_ = nullptr;
		rhs.d_ = nullptr;
	}
	CountConstructionDestruction &operator=(
		CountConstructionDestruction &&rhs
	) {
		c_ = rhs.c_;
		d_ = rhs.d_;
		rhs.c_ = nullptr;
		rhs.d_ = nullptr;
		return *this;
	}
	CountConstructionDestruction(
		std::atomic<size_t> *c, std::atomic<size_t> *d
	) : c_(c), d_(d) {
		c_->fetch_add(1);
	}
	~CountConstructionDestruction() {
		if (d_ != nullptr)
			d_->fetch_add(1);
	}
private:
	std::atomic<size_t> *c_;
	std::atomic<size_t> *d_;
};
void sp_memleak(size_t size) {
	std::atomic<size_t> c(0), d(0);
	{
		auto [sender, receiver] = channel<CountConstructionDestruction>(size);
		CountConstructionDestruction x(&c, &d);
		std::thread s(
			repetitive_send<CountConstructionDestruction>, std::move(sender),
			std::move(x), size
		);
		s.join();
	}
	ASSERT_EQ(c, d);
}

TEST(SP, MemLeak1) {
	sp_memleak(1);
}
// 2^20 == 1048576
TEST(SP, MemLeak2pow20) {
	sp_memleak(2 << 20);
}

template <typename T>
void mpsc_repetitive_send_recv(
	size_t num_senders, size_t msg_per_sender, size_t size, T x
) {
	std::vector<std::thread> ss;
	auto [sender, receiver] = channel<T>(size);
	for (size_t i = 0; i < num_senders; ++i) {
		ss.emplace_back(repetitive_send<T>, sender.clone(), x, msg_per_sender);
	}
	sender.drop();
	repetitive_recv(std::move(receiver), x, num_senders * msg_per_sender);
	for (std::thread &s : ss) {
		s.join();
	}
}

TEST(MPSC, RepetitiveSendRecvIntSender1MsgPerSender1e5Size1) {
	mpsc_repetitive_send_recv(1, 100000, 1, 233);
}
TEST(MPSC, RepetitiveSendRecvIntSender1MsgPerSender1e5Size2) {
	mpsc_repetitive_send_recv(1, 100000, 2, 233);
}
TEST(MPSC, RepetitiveSendRecvIntSender1MsgPerSender1e5Size4) {
	mpsc_repetitive_send_recv(1, 100000, 4, 233);
}
TEST(MPSC, RepetitiveSendRecvIntSender1MsgPerSender1e5Size8) {
	mpsc_repetitive_send_recv(1, 100000, 8, 233);
}
TEST(MPSC, RepetitiveSendRecvIntSender1MsgPerSender1e5Size16) {
	mpsc_repetitive_send_recv(1, 100000, 16, 233);
}
TEST(MPSC, RepetitiveSendRecvIntSender1MsgPerSender1e5Size32) {
	mpsc_repetitive_send_recv(1, 100000, 32, 233);
}
TEST(MPSC, RepetitiveSendRecvIntSender1MsgPerSender1e5Size64) {
	mpsc_repetitive_send_recv(1, 100000, 64, 233);
}
TEST(MPSC, RepetitiveSendRecvIntSender1MsgPerSender1e5Size2pow20) {
	mpsc_repetitive_send_recv(1, 100000, 1 << 20, 233);
}

TEST(MPSC, RepetitiveSendRecvIntSender2MsgPerSender1e5Size1) {
	mpsc_repetitive_send_recv(2, 100000, 1, 233);
}
TEST(MPSC, RepetitiveSendRecvIntSender2MsgPerSender1e5Size2) {
	mpsc_repetitive_send_recv(2, 100000, 2, 233);
}
TEST(MPSC, RepetitiveSendRecvIntSender2MsgPerSender1e5Size4) {
	mpsc_repetitive_send_recv(2, 100000, 4, 233);
}
TEST(MPSC, RepetitiveSendRecvIntSender2MsgPerSender1e5Size8) {
	mpsc_repetitive_send_recv(2, 100000, 8, 233);
}
TEST(MPSC, RepetitiveSendRecvIntSender2MsgPerSender1e5Size16) {
	mpsc_repetitive_send_recv(2, 100000, 16, 233);
}
TEST(MPSC, RepetitiveSendRecvIntSender2MsgPerSender1e5Size32) {
	mpsc_repetitive_send_recv(2, 100000, 32, 233);
}
TEST(MPSC, RepetitiveSendRecvIntSender2MsgPerSender1e5Size64) {
	mpsc_repetitive_send_recv(2, 100000, 64, 233);
}
TEST(MPSC, RepetitiveSendRecvIntSender2MsgPerSender1e5Size2pow20) {
	mpsc_repetitive_send_recv(2, 100000, 1 << 20, 233);
}

TEST(MPSC, RepetitiveSendRecvIntSender4MsgPerSender1e5Size2) {
	mpsc_repetitive_send_recv(4, 100000, 2, 233);
}
TEST(MPSC, RepetitiveSendRecvIntSender4MsgPerSender1e5Size4) {
	mpsc_repetitive_send_recv(4, 100000, 4, 233);
}
TEST(MPSC, RepetitiveSendRecvIntSender4MsgPerSender1e5Size8) {
	mpsc_repetitive_send_recv(4, 100000, 8, 233);
}
TEST(MPSC, RepetitiveSendRecvIntSender4MsgPerSender1e5Size2pow14) {
	mpsc_repetitive_send_recv(4, 100000, 1 << 16, 233);
}
TEST(MPSC, RepetitiveSendRecvIntSender4MsgPerSender1e5Size2pow15) {
	mpsc_repetitive_send_recv(4, 100000, 1 << 16, 233);
}
TEST(MPSC, RepetitiveSendRecvIntSender4MsgPerSender1e5Size2pow16) {
	mpsc_repetitive_send_recv(4, 100000, 1 << 16, 233);
}
TEST(MPSC, RepetitiveSendRecvIntSender4MsgPerSender1e5Size2pow17) {
	mpsc_repetitive_send_recv(4, 100000, 1 << 17, 233);
}
TEST(MPSC, RepetitiveSendRecvIntSender4MsgPerSender1e5Size2pow18) {
	mpsc_repetitive_send_recv(4, 100000, 1 << 18, 233);
}
TEST(MPSC, RepetitiveSendRecvIntSender4MsgPerSender1e5Size2pow19) {
	mpsc_repetitive_send_recv(4, 100000, 1 << 19, 233);
}
TEST(MPSC, RepetitiveSendRecvIntSender4MsgPerSender1e5Size2pow20) {
	mpsc_repetitive_send_recv(4, 100000, 1 << 20, 233);
}

TEST(MPSC, RepetitiveSendRecvIntSender8MsgPerSender1e5Size2) {
	mpsc_repetitive_send_recv(8, 100000, 2, 233);
}
TEST(MPSC, RepetitiveSendRecvIntSender8MsgPerSender1e5Size4) {
	mpsc_repetitive_send_recv(8, 100000, 4, 233);
}
TEST(MPSC, RepetitiveSendRecvIntSender8MsgPerSender1e5Size8) {
	mpsc_repetitive_send_recv(8, 100000, 8, 233);
}
TEST(MPSC, RepetitiveSendRecvIntSender8MsgPerSender1e5Size2pow20) {
	mpsc_repetitive_send_recv(8, 100000, 1 << 20, 233);
}

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
