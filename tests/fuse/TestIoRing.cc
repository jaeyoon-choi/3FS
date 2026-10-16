#include <gtest/gtest.h>
#include <memory>
#include <vector>

#include "fuse/IoRing.h"

namespace hf3fs::fuse {
namespace {

std::shared_ptr<IoRing> makeRing(std::vector<uint8_t> &buf, int entries, int ioDepth) {
  buf.assign(IoRing::bytesRequired(entries), 0);
  return std::make_shared<
      IoRing>(nullptr, "test", meta::UserInfo{}, true, buf.data(), buf.size(), ioDepth, 1, Duration::zero(), 0);
}

int fillSqes(IoRing &ring, int firstIndex) {
  int n = 0;
  while (ring.addSqe(firstIndex + n, nullptr)) {
    ++n;
  }
  return n;
}

TEST(TestIoRing, SqeSlotsFreedWhenJobsAreTaken) {
  std::vector<uint8_t> buf;
  auto ring = makeRing(buf, 8, 0);
  ASSERT_EQ(fillSqes(*ring, 0), 8);

  auto jobs = ring->jobsToProc(32);
  ASSERT_EQ(jobs.size(), 1);
  ASSERT_EQ(jobs[0].sqes->size(), 8);

  // The submitter can queue again while the job is still in flight.
  ASSERT_EQ(fillSqes(*ring, 100), 8);
  for (int i = 0; i < 8; ++i) {
    ASSERT_EQ((*jobs[0].sqes)[i].index, i);
  }
}

TEST(TestIoRing, SqeSlotsFreedPerJobWithIoDepth) {
  std::vector<uint8_t> buf;
  auto ring = makeRing(buf, 8, 3);
  ASSERT_EQ(fillSqes(*ring, 0), 8);

  // Two full batches of 3 are taken, the last 2 sqes wait for a full batch.
  auto jobs = ring->jobsToProc(32);
  ASSERT_EQ(jobs.size(), 2);
  for (int j = 0; j < 2; ++j) {
    ASSERT_EQ(jobs[j].sqes->size(), 3);
    for (int i = 0; i < 3; ++i) {
      ASSERT_EQ((*jobs[j].sqes)[i].index, j * 3 + i);
    }
  }

  // Only the 6 sqes owned by jobs are free again, and they wrap around the ring.
  ASSERT_EQ(fillSqes(*ring, 100), 6);
}

}  // namespace
}  // namespace hf3fs::fuse
