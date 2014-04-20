//-----------------------------------------------------------------------------
/** @brief MessagePackをデシリアライズする。
    @param[in] in_data   デシリアライズするMessagePackの先頭位置。
    @param[in] in_size   デシリアライズするMessagePackのバイト数。
    @param[out] out_root デシリアライズしたMessagePackの最上位要素。
 */
msgpack_unpack_return msgpack_unpack(
    char const* const in_data,
    std::size_t const in_size,
    std::size_t* const io_offset,
    msgpack_zone& out_zone,
    msgpack_object& out_root)
{
    std::size_t local_offset(0);
    if (io_offset != nullptr)
    {
        local_offset = *io_offset;
    }

    if (in_size <= local_offset)
    {
        return MSGPACK_UNPACK_CONTINUE; // FIXME
    }
    deserialize_context local_context;
    local_context.initialize(out_zone);

    int const local_execute(
        local_context.deserialize(in_data, in_size, local_offset));
    if (local_execute < 0)
    {
        return MSGPACK_UNPACK_PARSE_ERROR;
    }
    if (io_offset != nullptr)
    {
        *io_offset = local_offset;
    }

    if (local_execute == 0)
    {
        return MSGPACK_UNPACK_CONTINUE;
    }
    else
    {
        out_root = local_context.get_root_object();
        return local_offset < in_size?
            MSGPACK_UNPACK_EXTRA_BYTES: MSGPACK_UNPACK_SUCCESS;
    }
}
