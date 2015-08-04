/*
 * Copyright 2011 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
// Generated by j/c/g/ipc/invalidation/common/proto_wrapper_generator
package com.google.ipc.invalidation.ticl.proto;

import com.google.ipc.invalidation.util.Bytes;
import com.google.ipc.invalidation.util.ProtoWrapper;
import com.google.ipc.invalidation.util.ProtoWrapper.ValidationException;
import com.google.ipc.invalidation.util.TextBuilder;
import com.google.protobuf.nano.MessageNano;
import com.google.protobuf.nano.InvalidProtocolBufferNanoException;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;


public interface ChannelCommon {

  public static final class ChannelMessageEncoding extends ProtoWrapper {
    public interface MessageEncoding {
      public static final int PROTOBUF_BINARY_FORMAT = 1;
    }

    public static ChannelMessageEncoding create() {
      return new ChannelMessageEncoding();
    }

    public static final ChannelMessageEncoding DEFAULT_INSTANCE = new ChannelMessageEncoding();


    private ChannelMessageEncoding() {
    }


    @Override public final boolean equals(Object obj) {
      if (this == obj) { return true; }
      if (!(obj instanceof ChannelMessageEncoding)) { return false; }
      ChannelMessageEncoding other = (ChannelMessageEncoding) obj;
      return true;
    }

    @Override protected int computeHashCode() {
      int result = 1;
      return result;
    }

    @Override public void toCompactString(TextBuilder builder) {
      builder.append("<ChannelMessageEncoding:");
      builder.append('>');
    }

    public static ChannelMessageEncoding parseFrom(byte[] data) throws ValidationException {
      try {
        return fromMessageNano(MessageNano.mergeFrom(new com.google.protos.ipc.invalidation.NanoChannelCommon.ChannelMessageEncoding(), data));
      } catch (InvalidProtocolBufferNanoException exception) {
        throw new ValidationException(exception);
      } catch (ValidationArgumentException exception) {
        throw new ValidationException(exception.getMessage());
      }
    }

    static ChannelMessageEncoding fromMessageNano(com.google.protos.ipc.invalidation.NanoChannelCommon.ChannelMessageEncoding message) {
      if (message == null) { return null; }
      return new ChannelMessageEncoding();
    }

    public byte[] toByteArray() {
      return MessageNano.toByteArray(toMessageNano());
    }

    com.google.protos.ipc.invalidation.NanoChannelCommon.ChannelMessageEncoding toMessageNano() {
      com.google.protos.ipc.invalidation.NanoChannelCommon.ChannelMessageEncoding msg = new com.google.protos.ipc.invalidation.NanoChannelCommon.ChannelMessageEncoding();
      return msg;
    }
  }

  public static final class NetworkEndpointId extends ProtoWrapper {
    public interface NetworkAddress {
      public static final int TEST = 1;
      public static final int ANDROID = 113;
      public static final int LCS = 114;
    }

    public static NetworkEndpointId create(Integer networkAddress,
        Bytes clientAddress,
        Boolean isOffline) {
      return new NetworkEndpointId(networkAddress, clientAddress, isOffline);
    }

    public static final NetworkEndpointId DEFAULT_INSTANCE = new NetworkEndpointId(null, null, null);

    private final long __hazzerBits;
    private final int networkAddress;
    private final Bytes clientAddress;
    private final boolean isOffline;

    private NetworkEndpointId(Integer networkAddress,
        Bytes clientAddress,
        Boolean isOffline) {
      int hazzerBits = 0;
      if (networkAddress != null) {
        hazzerBits |= 0x1;
        this.networkAddress = networkAddress;
      } else {
        this.networkAddress = 1;
      }
      if (clientAddress != null) {
        hazzerBits |= 0x2;
        this.clientAddress = clientAddress;
      } else {
        this.clientAddress = Bytes.EMPTY_BYTES;
      }
      if (isOffline != null) {
        hazzerBits |= 0x4;
        this.isOffline = isOffline;
      } else {
        this.isOffline = false;
      }
      this.__hazzerBits = hazzerBits;
    }

    public int getNetworkAddress() { return networkAddress; }
    public boolean hasNetworkAddress() { return (0x1 & __hazzerBits) != 0; }

    public Bytes getClientAddress() { return clientAddress; }
    public boolean hasClientAddress() { return (0x2 & __hazzerBits) != 0; }

    public boolean getIsOffline() { return isOffline; }
    public boolean hasIsOffline() { return (0x4 & __hazzerBits) != 0; }

    @Override public final boolean equals(Object obj) {
      if (this == obj) { return true; }
      if (!(obj instanceof NetworkEndpointId)) { return false; }
      NetworkEndpointId other = (NetworkEndpointId) obj;
      return __hazzerBits == other.__hazzerBits
          && (!hasNetworkAddress() || networkAddress == other.networkAddress)
          && (!hasClientAddress() || equals(clientAddress, other.clientAddress))
          && (!hasIsOffline() || isOffline == other.isOffline);
    }

    @Override protected int computeHashCode() {
      int result = hash(__hazzerBits);
      if (hasNetworkAddress()) {
        result = result * 31 + hash(networkAddress);
      }
      if (hasClientAddress()) {
        result = result * 31 + clientAddress.hashCode();
      }
      if (hasIsOffline()) {
        result = result * 31 + hash(isOffline);
      }
      return result;
    }

    @Override public void toCompactString(TextBuilder builder) {
      builder.append("<NetworkEndpointId:");
      if (hasNetworkAddress()) {
        builder.append(" network_address=").append(networkAddress);
      }
      if (hasClientAddress()) {
        builder.append(" client_address=").append(clientAddress);
      }
      if (hasIsOffline()) {
        builder.append(" is_offline=").append(isOffline);
      }
      builder.append('>');
    }

    public static NetworkEndpointId parseFrom(byte[] data) throws ValidationException {
      try {
        return fromMessageNano(MessageNano.mergeFrom(new com.google.protos.ipc.invalidation.NanoChannelCommon.NetworkEndpointId(), data));
      } catch (InvalidProtocolBufferNanoException exception) {
        throw new ValidationException(exception);
      } catch (ValidationArgumentException exception) {
        throw new ValidationException(exception.getMessage());
      }
    }

    static NetworkEndpointId fromMessageNano(com.google.protos.ipc.invalidation.NanoChannelCommon.NetworkEndpointId message) {
      if (message == null) { return null; }
      return new NetworkEndpointId(message.networkAddress,
          Bytes.fromByteArray(message.clientAddress),
          message.isOffline);
    }

    public byte[] toByteArray() {
      return MessageNano.toByteArray(toMessageNano());
    }

    com.google.protos.ipc.invalidation.NanoChannelCommon.NetworkEndpointId toMessageNano() {
      com.google.protos.ipc.invalidation.NanoChannelCommon.NetworkEndpointId msg = new com.google.protos.ipc.invalidation.NanoChannelCommon.NetworkEndpointId();
      msg.networkAddress = hasNetworkAddress() ? networkAddress : null;
      msg.clientAddress = hasClientAddress() ? clientAddress.getByteArray() : null;
      msg.isOffline = hasIsOffline() ? isOffline : null;
      return msg;
    }
  }
}
