#ifndef MESSAGE_TYPES_H
#define MESSAGE_TYPES_H

#include <cstdint>
#include <vector>
#include <string>
#include <algorithm>
// #include <openssl/md5.h>
#include <qDebug>
enum class MessageType {
    Text,
    Binary,
    FilePath,
    FileList,
    Command,
    END
};

// struct Message {
//     MessageType type;
//     uint32_t chunk_id; // 块编号
//     uint32_t length; // 消息内容的长度
//     std::vector<char> data; // 消息内容
//     std::string filename; // 文件名
//     uint32_t filenameLength = filename.size();
//     unsigned char checksum[MD5_DIGEST_LENGTH]; // MD5校验和

//     // 计算MD5校验和
//     void calculateChecksum() {
//         MD5(reinterpret_cast<const unsigned char*>(data.data()), data.size(), checksum);
//     }

//     std::vector<char> serialize() const {
//         std::vector<char> result;
//         result.push_back(static_cast<char>(type));
//         result.insert(result.end(), reinterpret_cast<const char*>(&chunk_id), reinterpret_cast<const char*>(&chunk_id) + sizeof(chunk_id));
//         result.insert(result.end(), reinterpret_cast<const char*>(&filenameLength), reinterpret_cast<const char*>(&filenameLength) + sizeof(filenameLength));
//         result.insert(result.end(), filename.begin(), filename.end());
//         result.insert(result.end(), reinterpret_cast<const char*>(&length), reinterpret_cast<const char*>(&length) + sizeof(length));
//         result.insert(result.end(), data.begin(), data.end());
//         result.insert(result.end(), reinterpret_cast<const char*>(checksum), reinterpret_cast<const char*>(checksum) + MD5_DIGEST_LENGTH);
//         return result;
//     }

//     // 从字节流中反序列化出Message对象
//     static Message deserialize(const std::vector<char>& bytes) {
//         Message msg;
//         int offset = 0;
//         msg.type = static_cast<MessageType>(bytes[offset]);
//         offset += 1;

//         std::memcpy(&msg.chunk_id, &bytes[offset], sizeof(msg.chunk_id));
//         offset += sizeof(msg.chunk_id);

//         std::memcpy(&msg.filenameLength, &bytes[offset], sizeof(msg.filenameLength));
//         offset += sizeof(msg.filenameLength);

//         msg.filename = std::string(bytes.begin() + offset, bytes.begin() + offset + msg.filenameLength);
//         offset += msg.filenameLength;

//         std::memcpy(&msg.length, &bytes[offset], sizeof(msg.length));
//         offset += sizeof(msg.length);

//         msg.data = std::vector<char>(bytes.begin() + offset, bytes.begin() + offset + msg.length);
//         offset += msg.length;

//         std::memcpy(msg.checksum, &bytes[offset], MD5_DIGEST_LENGTH);

//         return msg;
//     }

//     // 验证校验和
//     bool validateChecksum() const {
//         unsigned char computed_checksum[MD5_DIGEST_LENGTH];
//         MD5(reinterpret_cast<const unsigned char*>(data.data()), data.size(), computed_checksum);
//         return std::memcmp(computed_checksum, checksum, MD5_DIGEST_LENGTH) == 0;
//     }
// };



struct Message {
    MessageType type;
    uint32_t length; // 消息内容的长度
    std::vector<char> data; // 消息内容
    std::string filename; // 文件名
    uint32_t filenameLength = filename.size();

    // 将Message对象序列化为字节流
    std::vector<char> serialize() const {
        std::vector<char> result;
        result.push_back(static_cast<char>(type));
        // 序列化文件名长度和文件名

        result.insert(result.end(), reinterpret_cast<const char*>(&filenameLength), reinterpret_cast<const char*>(&filenameLength) + sizeof(filenameLength));
        result.insert(result.end(), filename.begin(), filename.end());

        result.insert(result.end(), reinterpret_cast<const char*>(&length), reinterpret_cast<const char*>(&length) + sizeof(length));
        result.insert(result.end(), data.begin(), data.end());
        return result;
    }

    // 从字节流中反序列化出Message对象
    static Message deserialize(const std::vector<char>& bytes) {
        Message msg;
        int offset = 0;
        // 反序列化类型
        msg.type = static_cast<MessageType>(bytes[offset]);
        offset += sizeof(msg.type);
        // qDebug() << "type: "<<sizeof(msg.type)<<" | ";
        // qDebug() << "mike: "<<msg.filenameLength<<" | "<<sizeof(msg.filenameLength) <<" | "<<offset<<" | "<<bytes.size();
         std::memcpy(&msg.filenameLength, bytes.data() + offset, sizeof(msg.filenameLength));
        if(offset + sizeof(msg.filenameLength)<bytes.size()){

            msg.filename = std::string(bytes.begin() + offset+4, bytes.begin() + offset + sizeof(msg.filenameLength)+4);
            qDebug() << "data size:: "<<std::vector<char>(bytes.begin() , bytes.end());

            offset += sizeof(msg.filenameLength);
            qDebug() << "data length:: "<<msg.length<<" | "<<sizeof(msg.length)<<" | "<<bytes.end()-bytes.begin();
            std::memcpy(&msg.length, bytes.data() + offset, sizeof(msg.length));
            qDebug() << "data length:: "<<msg.length<<" | "<<sizeof(msg.length)<<" | "<<msg.filename;
            offset += sizeof(msg.length);
            msg.data = std::vector<char>(bytes.begin() + offset-3, bytes.end());
            // msg.length=bytes.end()-(bytes.begin() + offset-3);
        }
        return msg;
    }
};


#endif // MESSAGE_TYPES_H
