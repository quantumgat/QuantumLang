#include <cstdint>
#include <cstring>
#include <optional>
#include <span>

struct BinaryE2eUser {
  std::uint64_t id;
  std::uint8_t age;
  std::int32_t score;
};

struct BinaryE2eBuffer {
  std::uint8_t* data;
  std::size_t len;
};

static bool write_u64le(BinaryE2eBuffer buffer, std::size_t offset, std::uint64_t value) {
  if (offset + 8 > buffer.len) {
    return false;
  }
  std::memcpy(buffer.data + offset, &value, 8);
  return true;
}

static bool write_i32le(BinaryE2eBuffer buffer, std::size_t offset, std::int32_t value) {
  if (offset + 4 > buffer.len) {
    return false;
  }
  std::memcpy(buffer.data + offset, &value, 4);
  return true;
}

static std::optional<std::uint64_t> read_u64le(const std::uint8_t* data, std::size_t len, std::size_t offset) {
  if (offset + 8 > len) {
    return std::nullopt;
  }
  std::uint64_t value = 0;
  std::memcpy(&value, data + offset, 8);
  return value;
}

static std::optional<std::int32_t> read_i32le(const std::uint8_t* data, std::size_t len, std::size_t offset) {
  if (offset + 4 > len) {
    return std::nullopt;
  }
  std::int32_t value = 0;
  std::memcpy(&value, data + offset, 4);
  return value;
}

bool encode_binary_e2e_user(BinaryE2eBuffer buffer, BinaryE2eUser user) {
  if (!write_u64le(buffer, 0, user.id)) {
    return false;
  }
  if (9 > buffer.len) {
    return false;
  }
  buffer.data[8] = user.age;
  return write_i32le(buffer, 9, user.score);
}

std::optional<BinaryE2eUser> decode_binary_e2e_user(const std::uint8_t* data, std::size_t len) {
  auto id = read_u64le(data, len, 0);
  if (!id.has_value() || 9 > len) {
    return std::nullopt;
  }
  auto score = read_i32le(data, len, 9);
  if (!score.has_value()) {
    return std::nullopt;
  }
  return BinaryE2eUser{*id, data[8], *score};
}

struct BinaryPreparedCodec {
  std::size_t id_offset;
  std::size_t age_offset;
  std::size_t score_offset;
  std::size_t encoded_size;
};

static BinaryPreparedCodec prepare_binary_hotpath_codec() {
  return BinaryPreparedCodec{0, 8, 9, 13};
}

static std::optional<std::span<const std::uint8_t>> view_binary_hotpath_user(
  const std::uint8_t* data,
  std::size_t len,
  BinaryPreparedCodec codec
) {
  if (codec.encoded_size > len) {
    return std::nullopt;
  }
  return std::span<const std::uint8_t>(data, codec.encoded_size);
}

bool encode_binary_hotpath_user_into(
  BinaryE2eBuffer buffer,
  BinaryPreparedCodec codec,
  BinaryE2eUser user
) {
  if (codec.encoded_size > buffer.len) {
    return false;
  }
  if (!write_u64le(buffer, codec.id_offset, user.id)) {
    return false;
  }
  buffer.data[codec.age_offset] = user.age;
  return write_i32le(buffer, codec.score_offset, user.score);
}

std::optional<BinaryE2eUser> decode_binary_hotpath_user_prepared(
  const std::uint8_t* data,
  std::size_t len,
  BinaryPreparedCodec codec
) {
  auto view = view_binary_hotpath_user(data, len, codec);
  if (!view.has_value()) {
    return std::nullopt;
  }
  return decode_binary_e2e_user(view->data(), view->size());
}
