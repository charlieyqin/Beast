//------------------------------------------------------------------------------
/*
    This file is part of Beast: https://github.com/vinniefalco/Beast
    Copyright 2013, Vinnie Falco <vinnie.falco@gmail.com>

    Permission to use, copy, modify, and/or distribute this software for any
    purpose  with  or without fee is hereby granted, provided that the above
    copyright notice and this permission notice appear in all copies.

    THE  SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
    WITH  REGARD  TO  THIS  SOFTWARE  INCLUDING  ALL  IMPLIED  WARRANTIES  OF
    MERCHANTABILITY  AND  FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
    ANY  SPECIAL ,  DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
    WHATSOEVER  RESULTING  FROM  LOSS  OF USE, DATA OR PROFITS, WHETHER IN AN
    ACTION  OF  CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/
//==============================================================================

#ifndef BEAST_SOCKET_H_INCLUDED
#define BEAST_SOCKET_H_INCLUDED

/** A high level socket abstraction.

    This combines the capabilities of multiple socket interfaces such
    as listening, connecting, streaming, and handshaking. It brings
    everything together into a single abstract interface.

    When member functions are called and the underlying implementation does
    not support the operation, a fatal error is generated.

    Must satisfy these requirements:
    
        DefaultConstructible, MoveConstructible, CopyConstructible,
        MoveAssignable, CopyAssignable, Destructible

    Meets the requirements of these boost concepts:

        SyncReadStream, SyncWriteStream, AsyncReadStream, AsyncWriteStream,

    @see SharedObjectPtr
*/
class Socket
    : public SocketBase
    , public boost::asio::ssl::stream_base
    , public boost::asio::socket_base
{
public:
    virtual ~Socket () { }

    //--------------------------------------------------------------------------
    //
    // General attributes
    //
    //--------------------------------------------------------------------------

    /** Determines if the underlying stream requires a handshake.

        If is_handshaked is true, it will be necessary to call handshake or
        async_handshake after the connection is established. Furthermore it
        will be necessary to call the shutdown member from the
        HandshakeInterface to close the connection. Do not close the underlying
        socket or else the closure will not be graceful. Only one side should
        initiate the handshaking shutdon. The other side should observe it.
        Which side does what is up to the user.
    */
    virtual bool is_handshaked () = 0;

    /** Retrieve the underlying object.
        Returns nullptr if the implementation doesn't match. Usually
        you will use this if you need to get at the underlying boost::asio
        object. For example:

        @code

        void set_options (Socket& socket)
        {
            bost::boost::asio::ip::tcp::socket* sock =
                socket.native_object <bost::boost::asio::ip::tcp::socket> ();

            if (sock != nullptr)
                sock->set_option (
                    boost::boost::asio::ip::tcp::no_delay (true));
        }

        @endcode
    */
    template <class Object>
    Object* native_object ()
    {
        void* const object = native_object_raw ();
        if (object != nullptr)
            return dynamic_cast <Object> (object);
        return object;
    }

    virtual void* native_object_raw () = 0;

    //--------------------------------------------------------------------------
    //
    // SocketInterface
    //
    //--------------------------------------------------------------------------

    void cancel ()
    {
        boost::system::error_code ec;
        throw_error (cancel (ec));
    }

    virtual boost::system::error_code cancel (boost::system::error_code& ec) = 0;

    void shutdown (shutdown_type what)
    {
        boost::system::error_code ec;
        throw_error (shutdown (what, ec));
    }

    virtual boost::system::error_code shutdown (shutdown_type what, boost::system::error_code& ec) = 0;

    void close ()
    {
        boost::system::error_code ec;
        throw_error (close (ec));
    }

    virtual boost::system::error_code close (boost::system::error_code& ec) = 0;

    //--------------------------------------------------------------------------
    //
    // StreamInterface
    //
    //--------------------------------------------------------------------------

    // SyncReadStream
    //
    // http://www.boost.org/doc/libs/1_54_0/doc/html/boost_asio/reference/SyncReadStream.html
    //
    template <class MutableBufferSequence>
    std::size_t read_some (MutableBufferSequence const& buffers, boost::system::error_code& ec)
    {
        return read_some_impl (MutableBuffers (buffers), ec);
    }

    virtual std::size_t read_some_impl (MutableBuffers const& buffers, boost::system::error_code& ec) = 0;

    // SyncWriteStream
    //
    // http://www.boost.org/doc/libs/1_54_0/doc/html/boost_asio/reference/SyncWriteStream.html
    //
    template <class ConstBufferSequence>
    std::size_t write_some (ConstBufferSequence const& buffers, boost::system::error_code &ec)
    {
        return write_some_impl (ConstBuffers (buffers), ec);
    }

    virtual std::size_t write_some_impl (ConstBuffers const& buffers, boost::system::error_code& ec) = 0;

    // AsyncReadStream
    //
    // http://www.boost.org/doc/libs/1_54_0/doc/html/boost_asio/reference/AsyncReadStream.html
    //
    template <class MutableBufferSequence, class ReadHandler>
    void async_read_some (MutableBufferSequence const& buffers,
        BOOST_ASIO_MOVE_ARG(ReadHandler) handler)
    {
        async_read_some_impl (MutableBuffers (buffers), handler);
    }

    virtual void async_read_some_impl (MutableBuffers const& buffers, TransferCall const& call) = 0;

    // AsyncWriteStream
    //
    // http://www.boost.org/doc/libs/1_54_0/doc/html/boost_asio/reference/AsyncWriteStream.html
    //
    template <class ConstBufferSequence, class WriteHandler>
    void async_write_some (ConstBufferSequence const& buffers,
        BOOST_ASIO_MOVE_ARG(WriteHandler) handler)
    {
        async_write_some_impl (ConstBuffers (buffers), handler);
    }

    virtual void async_write_some_impl (ConstBuffers const& buffers, TransferCall const& call) = 0;

    //--------------------------------------------------------------------------
    //
    // HandshakeInterface
    //
    //--------------------------------------------------------------------------

    // ssl::stream::handshake (1 of 4)
    // http://www.boost.org/doc/libs/1_54_0/doc/html/boost_asio/reference/ssl__stream/handshake/overload1.html
    //
    void handshake (handshake_type role)
    {
        boost::system::error_code ec;
        throw_error (handshake (role, ec));
    }

    // ssl::stream::handshake (2 of 4)
    // http://www.boost.org/doc/libs/1_54_0/doc/html/boost_asio/reference/ssl__stream/handshake/overload2.html
    //
    virtual boost::system::error_code handshake (handshake_type role, boost::system::error_code& ec) = 0;

#if (BOOST_VERSION / 100) >= 1054
    // ssl::stream::handshake (3 of 4)
    // http://www.boost.org/doc/libs/1_54_0/doc/html/boost_asio/reference/ssl__stream/handshake/overload3.html
    //
    template <class ConstBufferSequence>
    void handshake (handshake_type role, ConstBufferSequence const& buffers)
    {
        boost::system::error_code ec;
        throw_error (handshake (role, buffers, ec));
    }

    // ssl::stream::handshake (4 of 4)
    // http://www.boost.org/doc/libs/1_54_0/doc/html/boost_asio/reference/ssl__stream/handshake/overload4.html
    //
    template <class ConstBufferSequence>
    boost::system::error_code handshake (handshake_type role,
        ConstBufferSequence const& buffers, boost::system::error_code& ec)
    {
        return handshake_impl (role, ConstBuffers (buffers), ec);
    }

    virtual boost::system::error_code handshake_impl (handshake_type role,
        ConstBuffers const& buffers, boost::system::error_code& ec) = 0;
#endif

    // ssl::stream::async_handshake (1 of 2)
    // http://www.boost.org/doc/libs/1_54_0/doc/html/boost_asio/reference/ssl__stream/async_handshake/overload1.html
    //
    template <class HandshakeHandler>
    void async_handshake (handshake_type role, BOOST_ASIO_MOVE_ARG(HandshakeHandler) handler)
    {
        async_handshake_impl (role, handler);
    }

    virtual void async_handshake_impl (handshake_type role, ErrorCall const& calll) = 0;

#if (BOOST_VERSION / 100) >= 1054
    // ssl::stream::async_handshake (2 of 2)
    // http://www.boost.org/doc/libs/1_54_0/doc/html/boost_asio/reference/ssl__stream/async_handshake/overload2.html
    //
    template <class ConstBufferSequence, class BufferedHandshakeHandler>
    void async_handshake (handshake_type role, ConstBufferSequence const& buffers,
        BOOST_ASIO_MOVE_ARG(BufferedHandshakeHandler) handler)
    {
        async_handshake_impl (role, ConstBuffers (buffers), handler);
    }

    virtual void async_handshake_impl (handshake_type role,
        ConstBuffers const& buffers, TransferCall const& call) = 0;
#endif

    // ssl::stream::shutdown
    // http://www.boost.org/doc/libs/1_54_0/doc/html/boost_asio/reference/ssl__stream/shutdown.html
    //
    void shutdown ()
    {
        boost::system::error_code ec;
        throw_error (shutdown (ec));
    }

    virtual boost::system::error_code shutdown (boost::system::error_code& ec) = 0;

    // ssl::stream::async_shutdown
    // http://www.boost.org/doc/libs/1_54_0/doc/html/boost_asio/reference/ssl__stream/async_shutdown.html
    //
    template <class ShutdownHandler>
    void async_shutdown (BOOST_ASIO_MOVE_ARG(ShutdownHandler) handler)
    {
        async_shutdown_impl (handler);
    }

    virtual void async_shutdown_impl (ErrorCall const& call) = 0;
};

#endif