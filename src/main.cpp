#include <gtest/gtest.h>
#include <mpsc_ring.hpp>
#include <thread>

using mpsc_ring::Sender;
using mpsc_ring::Receiver;
using mpsc_ring::channel;

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

class IncrWhenDestruct {
public:
	IncrWhenDestruct(const IncrWhenDestruct &rhs) : cnt_(rhs.cnt_) {}
	IncrWhenDestruct &operator=(const IncrWhenDestruct &rhs) {
		if (this == &rhs)
			return *this;
		this->~IncrWhenDestruct();
		cnt_ = rhs.cnt_;
		return *this;
	}
	IncrWhenDestruct(IncrWhenDestruct &&rhs) : cnt_(rhs.cnt_) {
		rhs.cnt_ = nullptr;
	}
	IncrWhenDestruct &operator=(IncrWhenDestruct &&rhs) {
		cnt_ = rhs.cnt_;
		rhs.cnt_ = nullptr;
		return *this;
	}
	IncrWhenDestruct(std::atomic<size_t> *cnt) : cnt_(cnt) {}
	~IncrWhenDestruct() {
		if (cnt_ != nullptr)
			cnt_->fetch_add(1);
	}
private:
	std::atomic<size_t> *cnt_;
};
void sp_memleak(size_t size) {
	std::atomic<size_t> cnt(0);
	{
		auto [sender, receiver] = channel<IncrWhenDestruct>(size);
		IncrWhenDestruct x(&cnt);
		std::thread s(
			repetitive_send<IncrWhenDestruct>, std::move(sender), std::move(x),
			size
		);
		s.join();
	}
	ASSERT_EQ(cnt, size + 1);
}

TEST(SP, MemLeak1) {
	sp_memleak(1);
}
// 2^20 == 1048576
TEST(SP, MemLeak2pow20) {
	sp_memleak(2 << 20);
}

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
