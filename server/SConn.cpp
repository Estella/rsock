//
// Created on 12/16/17.
//

#include <cassert>

#include "plog/Log.h"

#include "SConn.h"
#include "../util/rsutil.h"
#include "../conn/CConn.h"

SConn::SConn(const std::string &key, uv_loop_t *loop, const SA *target, uint32_t conv) : IConn(key) {
    assert(target->sa_family == AF_INET);

    mTarget = reinterpret_cast<SA4 *>(new_addr(target));
    mUdp = om_new_udp(loop, this, udpRecvCb);

    mConv = conv;
}

int SConn::Init() {
    IConn::Init();

    return 0;
}

int SConn::Close() {
    IConn::Close();

    if (mTarget) {
        free(mTarget);
        mTarget = nullptr;
    }

    if (mSelfAddr) {
        free(mSelfAddr);
        mSelfAddr = nullptr;
    }

    if (mUdp) {
        uv_close(reinterpret_cast<uv_handle_t *>(mUdp), close_cb);
        mUdp = nullptr;
    }
    return 0;
}

void SConn::udpRecvCb(uv_udp_t *handle, ssize_t nread, const uv_buf_t *buf, const SA *addr, unsigned flags) {
    SConn *conn = static_cast<SConn *>(handle->data);
    if (nread > 0) {
        if (nullptr == conn->mSelfAddr) {
            int socklen = sizeof(SA4);
            conn->mSelfAddr = static_cast<SA4 *>(malloc(sizeof(SA4)));
            conn->SetPrintableStr(CConn::BuildPrintableStr((SA *) conn->mSelfAddr));

            memset(conn->mSelfAddr, 0, sizeof(SA4));
            int err = uv_udp_getsockname(handle, reinterpret_cast<SA *>(conn->mSelfAddr), &socklen);
            if (err) {
                LOGE << "getsockname failed, err " << err << ": " << strerror(errno);
                assert(0);
            }
            LOGV << "sconn addr: " << Addr2Str((SA *) conn->mSelfAddr);
        }
//        LOGV << "receive " << nread << " bytes from " << Addr2Str(addr);
        const rbuf_t rbuf = new_buf(nread, buf->base, conn);
        conn->Send(nread, rbuf);
    } else if (nread < 0) {
        LOGE << "receive error: " << uv_strerror(nread);
    }
    free(buf->base);
}

uint32_t SConn::Conv() {
    return mConv;
}

int SConn::OnRecv(ssize_t nread, const rbuf_t &rbuf) {
    if (nread > 0) {
        rudp_send_t *udp_send = static_cast<rudp_send_t *>(malloc(sizeof(rudp_send_t)));
        memset(udp_send, 0, sizeof(rudp_send_t));
        char *base = static_cast<char *>(malloc(nread));
        memcpy(base, rbuf.base, nread);
        udp_send->buf = uv_buf_init(base, nread);
        udp_send->udp_send.data = this;
        uv_udp_send(reinterpret_cast<uv_udp_send_t *>(udp_send), mUdp, &udp_send->buf, 1,
                    reinterpret_cast<const SA *>(mTarget), sendCb);
    }
    return nread;
}

void SConn::sendCb(uv_udp_send_t *req, int status) {
    rudp_send_t *udp = reinterpret_cast<rudp_send_t *>(req);
    SConn *conn = static_cast<SConn *>(udp->udp_send.data);
    if (status) {
//        todo: add err processing
        LOGE << "udp send error, err " << status << ": " << uv_strerror(status);
    }

    free_rudp_send(udp);
}
