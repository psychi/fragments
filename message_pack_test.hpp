/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief
 */
#ifndef PSYQ_MESSAGE_PACK_TEST_HPP_
#define PSYQ_MESSAGE_PACK_TEST_HPP_

namespace psyq
{
    namespace test
    {
        inline void message_pack()
        {
            psyq::message_pack::serializer<std::stringstream, 16>
                local_serializer;
            std::unordered_set<int> local_integer_set;
            while (local_integer_set.size() < 0x10000)
            {
                local_integer_set.insert(local_integer_set.size());
            }
            std::string local_string_0x10("0123456789ABCDEF");
            std::string local_string_0x10000;
            local_string_0x10000.reserve(0x10000);
            while (local_string_0x10000.size() < 0x10000)
            {
                local_string_0x10000 += local_string_0x10;
            }
            std::string local_string_0x1f(local_string_0x10000, 0, 0x1f);
            std::string local_string_0xff(local_string_0x10000, 0, 0xff);
            std::string local_string_0xffff(local_string_0x10000, 0, 0xffff);

            local_serializer.make_serial_array(31);
            local_serializer.write_container_binary(
                local_integer_set.begin(), local_integer_set.size());
            local_serializer.write_extended(
                0x7f, 0x123456789abcdefLL, psyq::message_pack::little_endian);
            local_serializer.write_array(local_integer_set);
            local_serializer.write_set(local_integer_set);
            local_serializer.write_tuple(std::make_tuple(0, 0.0f, 0.0, false));
            local_serializer << (std::numeric_limits<std::int64_t>::min)();
            local_serializer << (std::numeric_limits<std::int32_t>::min)();
            local_serializer << (std::numeric_limits<std::int16_t>::min)();
            local_serializer << (std::numeric_limits<std::int8_t >::min)();
            local_serializer << -0x20;
            local_serializer << false;
            local_serializer << 0.25;
            local_serializer << 0.5f;
            local_serializer << true;
            local_serializer << int(0x7f);
            local_serializer << (std::numeric_limits<std::uint8_t >::max)();
            local_serializer << (std::numeric_limits<std::uint16_t>::max)();
            local_serializer << (std::numeric_limits<std::uint32_t>::max)();
            local_serializer << (std::numeric_limits<std::uint64_t>::max)();
            local_serializer << local_string_0x1f;
            local_serializer << local_string_0xff;
            local_serializer << local_string_0xffff;
            local_serializer << local_string_0x10000;
            local_serializer.write_container_binary(local_string_0xff);
            local_serializer.write_container_binary(local_string_0xffff);
            local_serializer.write_container_binary(local_string_0x10000);
            local_serializer.make_serial_extended(5, local_string_0x10.size());
            local_serializer.fill_container_raw(local_string_0x10);
            local_serializer.make_serial_extended(8, local_string_0xff.size());
            local_serializer.fill_container_raw(local_string_0xff);
            local_serializer.make_serial_extended(16, local_string_0xffff.size());
            local_serializer.fill_container_raw(local_string_0xffff);
            local_serializer.make_serial_extended(17, local_string_0x10000.size());
            local_serializer.fill_container_raw(local_string_0x10000);
            local_serializer.write_nil();

            std::stringstream local_stream;
            local_serializer.swap_stream(local_stream);
            local_stream.seekg(0);
            psyq::message_pack::deserializer<std::stringstream>
                local_deserializer(std::move(local_stream));
            psyq::message_pack::root_object<> local_root_object;
            local_deserializer >> local_root_object;
            auto local_message_pack_object(local_root_object.get_array()->data());
            PSYQ_ASSERT(local_message_pack_object != nullptr);
            ++local_message_pack_object;
            ++local_message_pack_object;
            ++local_message_pack_object;
            ++local_message_pack_object;
            ++local_message_pack_object;

            PSYQ_ASSERT(
                *local_message_pack_object
                    == psyq::message_pack::object(
                        (std::numeric_limits<std::int64_t>::min)()));
            ++local_message_pack_object;

            PSYQ_ASSERT(
                *local_message_pack_object
                    == psyq::message_pack::object(
                        (std::numeric_limits<std::int32_t>::min)()));
            ++local_message_pack_object;

            PSYQ_ASSERT(
                *local_message_pack_object
                    == psyq::message_pack::object(
                        (std::numeric_limits<std::int16_t>::min)()));
            ++local_message_pack_object;

            PSYQ_ASSERT(
                *local_message_pack_object
                    == psyq::message_pack::object(
                        (std::numeric_limits<std::int8_t>::min)()));
            ++local_message_pack_object;

            PSYQ_ASSERT(
                *local_message_pack_object
                    == psyq::message_pack::object(-0x20));
            ++local_message_pack_object;

            PSYQ_ASSERT(
                *local_message_pack_object
                    == psyq::message_pack::object(false));
            ++local_message_pack_object;

            PSYQ_ASSERT(
                *local_message_pack_object
                    == psyq::message_pack::object(0.25));
            ++local_message_pack_object;

            PSYQ_ASSERT(
                *local_message_pack_object
                    == psyq::message_pack::object(0.5f));
            ++local_message_pack_object;

            PSYQ_ASSERT(
                *local_message_pack_object
                    == psyq::message_pack::object(true));
            ++local_message_pack_object;

            PSYQ_ASSERT(
                *local_message_pack_object
                    == psyq::message_pack::object(0x7f));
            ++local_message_pack_object;

            PSYQ_ASSERT(
                *local_message_pack_object
                    == psyq::message_pack::object(
                        (std::numeric_limits<std::uint8_t>::max)()));
            ++local_message_pack_object;

            PSYQ_ASSERT(
                *local_message_pack_object
                    == psyq::message_pack::object(
                        (std::numeric_limits<std::uint16_t>::max)()));
            ++local_message_pack_object;

            PSYQ_ASSERT(
                *local_message_pack_object
                    == psyq::message_pack::object(
                        (std::numeric_limits<std::uint32_t>::max)()));
            ++local_message_pack_object;

            PSYQ_ASSERT(
                *local_message_pack_object
                    == psyq::message_pack::object(
                        (std::numeric_limits<std::uint64_t>::max)()));
            ++local_message_pack_object;

            PSYQ_ASSERT(
                local_string_0x1f == std::string(
                    local_message_pack_object->get_string()->begin(),
                    local_message_pack_object->get_string()->end()));
            ++local_message_pack_object;

            PSYQ_ASSERT(
                local_string_0xff == std::string(
                    local_message_pack_object->get_string()->begin(),
                    local_message_pack_object->get_string()->end()));
            ++local_message_pack_object;

            PSYQ_ASSERT(
                local_string_0xffff == std::string(
                        local_message_pack_object->get_string()->begin(),
                        local_message_pack_object->get_string()->end()));
            ++local_message_pack_object;

            PSYQ_ASSERT(
                local_string_0x10000 == std::string(
                    local_message_pack_object->get_string()->begin(),
                    local_message_pack_object->get_string()->end()));
            ++local_message_pack_object;

            PSYQ_ASSERT(
                local_string_0xff == std::string(
                    (char const*)local_message_pack_object->get_binary()->begin(),
                    (char const*)local_message_pack_object->get_binary()->end()));
            ++local_message_pack_object;

            PSYQ_ASSERT(
                local_string_0xffff == std::string(
                    (char const*)local_message_pack_object->get_binary()->begin(),
                    (char const*)local_message_pack_object->get_binary()->end()));
            ++local_message_pack_object;

            PSYQ_ASSERT(
                local_string_0x10000 == std::string(
                    (char const*)local_message_pack_object->get_binary()->begin(),
                    (char const*)local_message_pack_object->get_binary()->end()));
            ++local_message_pack_object;

            PSYQ_ASSERT(
                local_string_0x10 == std::string(
                    (char const*)local_message_pack_object->get_extended()->begin(),
                    (char const*)local_message_pack_object->get_extended()->end()));
            PSYQ_ASSERT(local_message_pack_object->get_extended()->type() == 5);
            ++local_message_pack_object;

            PSYQ_ASSERT(
                local_string_0xff == std::string(
                    (char const*)local_message_pack_object->get_extended()->begin(),
                    (char const*)local_message_pack_object->get_extended()->end()));
            PSYQ_ASSERT(local_message_pack_object->get_extended()->type() == 8);
            ++local_message_pack_object;

            PSYQ_ASSERT(
                local_string_0xffff == std::string(
                    (char const*)local_message_pack_object->get_extended()->begin(),
                    (char const*)local_message_pack_object->get_extended()->end()));
            PSYQ_ASSERT(local_message_pack_object->get_extended()->type() == 16);
            ++local_message_pack_object;

            PSYQ_ASSERT(
                local_string_0x10000 == std::string(
                    (char const*)local_message_pack_object->get_extended()->begin(),
                    (char const*)local_message_pack_object->get_extended()->end()));
            PSYQ_ASSERT(local_message_pack_object->get_extended()->type() == 17);
            ++local_message_pack_object;

            PSYQ_ASSERT(
                *local_message_pack_object == psyq::message_pack::object());
            ++local_message_pack_object;
        }
    } // namespace test
} // namespace psyq

#endif // PSYQ_MESSAGE_PACK_TEST_HPP_
