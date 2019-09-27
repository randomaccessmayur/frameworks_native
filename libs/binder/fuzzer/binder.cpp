/*
 * Copyright (C) 2019 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#define FUZZ_LOG_TAG "binder"

#include "binder.h"
#include "util.h"

using ::android::status_t;

#define PARCEL_READ_WITH_STATUS(T, FUN) \
    [] (const ::android::Parcel& p, uint8_t /*data*/) {\
        FUZZ_LOG() << "about to read " #T " using " #FUN " with status";\
        T t{};\
        status_t status = p.FUN(&t);\
        FUZZ_LOG() << #T " status: " << status /* << " value: " << t*/;\
    }

#define PARCEL_READ_NO_STATUS(T, FUN) \
    [] (const ::android::Parcel& p, uint8_t /*data*/) {\
        FUZZ_LOG() << "about to read " #T " using " #FUN " with no status";\
        T t = p.FUN();\
        (void) t;\
        FUZZ_LOG() << #T " done " /* << " value: " << t*/;\
    }

#define PARCEL_READ_OPT_STATUS(T, FUN) \
    PARCEL_READ_WITH_STATUS(T, FUN), \
    PARCEL_READ_NO_STATUS(T, FUN)

std::vector<ParcelRead<::android::Parcel>> BINDER_PARCEL_READ_FUNCTIONS {
    PARCEL_READ_NO_STATUS(size_t, dataSize),
    PARCEL_READ_NO_STATUS(size_t, dataAvail),
    PARCEL_READ_NO_STATUS(size_t, dataPosition),
    PARCEL_READ_NO_STATUS(size_t, dataCapacity),
    [] (const ::android::Parcel& p, uint8_t pos) {
        FUZZ_LOG() << "about to setDataPosition: " << pos;
        p.setDataPosition(pos);
        FUZZ_LOG() << "setDataPosition done";
    },
    PARCEL_READ_NO_STATUS(size_t, allowFds),
    PARCEL_READ_NO_STATUS(size_t, hasFileDescriptors),
    [] (const ::android::Parcel& p, uint8_t len) {
#ifdef __ANDROID__
        std::string interface(len, 'a');
        FUZZ_LOG() << "about to enforceInterface: " << interface;
        bool b = p.enforceInterface(::android::String16(interface.c_str()));
        FUZZ_LOG() << "enforced interface: " << b;
#else
        FUZZ_LOG() << "skipping enforceInterface";
        (void)p;
        (void)len;
#endif // __ANDROID__
    },
    [] (const ::android::Parcel& p, uint8_t /*len*/) {
#ifdef __ANDROID__
        FUZZ_LOG() << "about to checkInterface";
        bool b = p.checkInterface(new android::BBinder());
        FUZZ_LOG() << "checked interface: " << b;
#else
        FUZZ_LOG() << "skipping checkInterface";
        (void)p;
#endif // __ANDROID__
    },
    PARCEL_READ_NO_STATUS(size_t, objectsCount),
    PARCEL_READ_NO_STATUS(status_t, errorCheck),
    [] (const ::android::Parcel& p, uint8_t len) {
        FUZZ_LOG() << "about to read void*";
        std::vector<uint8_t> data(len);
        status_t status = p.read(data.data(), len);
        FUZZ_LOG() << "read status: " << status;
    },
    [] (const ::android::Parcel& p, uint8_t len) {
        FUZZ_LOG() << "about to readInplace";
        const void* r = p.readInplace(len);
        FUZZ_LOG() << "readInplace done. pointer: " << r;
    },
    PARCEL_READ_OPT_STATUS(int32_t, readInt32),
    PARCEL_READ_OPT_STATUS(uint32_t, readUint32),
    PARCEL_READ_OPT_STATUS(int64_t, readInt64),
    PARCEL_READ_OPT_STATUS(uint64_t, readUint64),
    PARCEL_READ_OPT_STATUS(float, readFloat),
    PARCEL_READ_OPT_STATUS(double, readDouble),
    PARCEL_READ_OPT_STATUS(intptr_t, readIntPtr),
    PARCEL_READ_OPT_STATUS(bool, readBool),
    PARCEL_READ_OPT_STATUS(char16_t, readChar),
    PARCEL_READ_OPT_STATUS(int8_t, readByte),

    PARCEL_READ_WITH_STATUS(std::string, readUtf8FromUtf16),
    PARCEL_READ_WITH_STATUS(std::unique_ptr<std::string>, readUtf8FromUtf16),
    [] (const ::android::Parcel& p, uint8_t /*data*/) {
        FUZZ_LOG() << "about to read c-str";
        const char* str = p.readCString();
        FUZZ_LOG() << "read c-str: " << (str ? str : "<empty string>");
    },
    PARCEL_READ_OPT_STATUS(android::String8, readString8),
    PARCEL_READ_OPT_STATUS(android::String16, readString16),
    PARCEL_READ_WITH_STATUS(std::unique_ptr<android::String16>, readString16),
    // TODO: readString16Inplace
    PARCEL_READ_WITH_STATUS(android::sp<android::IBinder>, readStrongBinder),
    PARCEL_READ_WITH_STATUS(android::sp<android::IBinder>, readNullableStrongBinder),

    // TODO: all templated versions of readParcelableVector, readParcelable
    // TODO: readParcelable
    // TODO: templated versions of readStrongBinder, readNullableStrongBinder

    // TODO(b/131868573): can force read of arbitrarily sized vector
    // PARCEL_READ_WITH_STATUS(::std::unique_ptr<std::vector<android::sp<android::IBinder>>>, readStrongBinderVector),
    // PARCEL_READ_WITH_STATUS(std::vector<android::sp<android::IBinder>>, readStrongBinderVector),

    // TODO(b/131868573): can force read of arbitrarily sized vector
    // PARCEL_READ_WITH_STATUS(std::unique_ptr<std::vector<int8_t>>, readByteVector),
    // PARCEL_READ_WITH_STATUS(std::vector<int8_t>, readByteVector),
    // PARCEL_READ_WITH_STATUS(std::unique_ptr<std::vector<uint8_t>>, readByteVector),
    // PARCEL_READ_WITH_STATUS(std::vector<uint8_t>, readByteVector),
    // PARCEL_READ_WITH_STATUS(std::unique_ptr<std::vector<int32_t>>, readInt32Vector),
    // PARCEL_READ_WITH_STATUS(std::vector<int32_t>, readInt32Vector),
    // PARCEL_READ_WITH_STATUS(std::unique_ptr<std::vector<int64_t>>, readInt64Vector),
    // PARCEL_READ_WITH_STATUS(std::vector<int64_t>, readInt64Vector),
    // PARCEL_READ_WITH_STATUS(std::unique_ptr<std::vector<uint64_t>>, readUint64Vector),
    // PARCEL_READ_WITH_STATUS(std::vector<uint64_t>, readUint64Vector),
    // PARCEL_READ_WITH_STATUS(std::unique_ptr<std::vector<float>>, readFloatVector),
    // PARCEL_READ_WITH_STATUS(std::vector<float>, readFloatVector),
    // PARCEL_READ_WITH_STATUS(std::unique_ptr<std::vector<double>>, readDoubleVector),
    // PARCEL_READ_WITH_STATUS(std::vector<double>, readDoubleVector),
    // PARCEL_READ_WITH_STATUS(std::unique_ptr<std::vector<bool>>, readBoolVector),
    // PARCEL_READ_WITH_STATUS(std::vector<bool>, readBoolVector),
    // PARCEL_READ_WITH_STATUS(std::unique_ptr<std::vector<char16_t>>, readCharVector),
    // PARCEL_READ_WITH_STATUS(std::vector<char16_t>, readCharVector),
    // PARCEL_READ_WITH_STATUS(std::unique_ptr<std::vector<std::unique_ptr<android::String16>>>, readString16Vector),
    // PARCEL_READ_WITH_STATUS(std::vector<android::String16>, readString16Vector),
    // PARCEL_READ_WITH_STATUS(std::unique_ptr<std::vector<std::unique_ptr<std::string>>>, readUtf8VectorFromUtf16Vector),
    // PARCEL_READ_WITH_STATUS(std::vector<std::string>, readUtf8VectorFromUtf16Vector),

    // TODO: read(Flattenable<T>)
    // TODO: read(LightFlattenable<T>)
    // TODO: resizeOutVector

    PARCEL_READ_NO_STATUS(int32_t, readExceptionCode),
    // TODO: readNativeHandle
    PARCEL_READ_NO_STATUS(int, readFileDescriptor),
    PARCEL_READ_NO_STATUS(int, readParcelFileDescriptor),
    PARCEL_READ_WITH_STATUS(android::base::unique_fd, readUniqueFileDescriptor),

    // TODO(b/131868573): can force read of arbitrarily sized vector
    // PARCEL_READ_WITH_STATUS(std::unique_ptr<std::vector<android::base::unique_fd>>, readUniqueFileDescriptorVector),
    // PARCEL_READ_WITH_STATUS(std::vector<android::base::unique_fd>, readUniqueFileDescriptorVector),

    [] (const android::Parcel& p, uint8_t len) {
        FUZZ_LOG() << "about to readBlob";
        ::android::Parcel::ReadableBlob blob;
        status_t status = p.readBlob(len, &blob);
        FUZZ_LOG() << "readBlob status: " << status;
    },
    // TODO: readObject
    PARCEL_READ_NO_STATUS(uid_t, readCallingWorkSourceUid),
    PARCEL_READ_NO_STATUS(size_t, getBlobAshmemSize),
    PARCEL_READ_NO_STATUS(size_t, getOpenAshmemSize),
};