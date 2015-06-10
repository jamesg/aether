#include "eeprom.hpp"

#include <Arduino.h>
#include "Wire.h"

#define MAX(a, b) (a > b) ? a : b

i2c_eeprom::address::address(char *start) :
    m_start(start)
{
}

i2c_eeprom::address& i2c_eeprom::address::operator=(address_type addr)
{
    m_start[0] = (unsigned char)(addr >> 8);
    m_start[1] = (unsigned char)(addr & 0xff);
}

i2c_eeprom::address::operator address_type() const
{
    return ((unsigned int)(m_start[0]) << 8) + (((unsigned int)m_start[1]) & 0xff);
}

i2c_eeprom::i2c_eeprom(const int i2c_id) :
    m_id(i2c_id)
{
}

i2c_eeprom::address_type i2c_eeprom::block_count() const
{
    // TODO read the size of the EEPROM (if possible).
    return BLOCK_COUNT;
}

void i2c_eeprom::write(
        unsigned int eeaddress,
        char *data,
        unsigned int data_len
        )
{
    char buf[16];
    snprintf(buf, sizeof(buf), "write 0x%x", eeaddress);
    Serial.println(buf);
    snprintf(buf, sizeof(buf), "len 0x%x", data_len);
    Serial.println(buf);

    // Remaining space to write in the first page.  Write this many bytes to
    // the first page, then move on to the next page.
    const unsigned int first_write_size = int(((eeaddress / 64) + 1) * 64) - eeaddress;
    snprintf(buf, sizeof(buf), "first_w 0x%x", first_write_size);
    Serial.println(buf);

    // Number of bytes to write to the last page.
    unsigned char last_write_size = (data_len - first_write_size) % 64;

    // Number of writes that must be made.
    unsigned int num_writes = (data_len > first_write_size) ?
        // (no. writes not including first or last) + 2
        ((data_len - first_write_size ) / 64) + 2 :
        1;
    snprintf(buf, sizeof(buf), "num 0x%x", num_writes);
    Serial.println(buf);

    // Start of next section to write to.
    unsigned int address = eeaddress;
    // data iterator.
    int i = 0;
    for(int page = 0; page < num_writes; ++page)
    {
        unsigned int write_size =
            (page == 0) ? first_write_size :
            (page == (num_writes - 1)) ? last_write_size :
            64;

        snprintf(buf, sizeof(buf), "bw 0x%x", write_size);
        Serial.println(buf);
        Wire.beginTransmission(m_id);
        Wire.write((address) >> 8);
        Wire.write((address) & 0xFF);

        for(int i_c = 0; i_c < write_size; ++i_c)
            Wire.write((byte)data[i++]);

        Wire.endTransmission();
        address += write_size;
        // Maximum page write time is 5ms.
        delay(6);
    }
}

void i2c_eeprom::read(
        unsigned int eeaddress,
        char *data,
        unsigned int data_len
        ) const
{
    char buf[16];
    snprintf(buf, sizeof(buf), "read 0x%x", eeaddress);
    Serial.println(buf);
    snprintf(buf, sizeof(buf), "high 0x%x", (unsigned char)(eeaddress >> 8));
    Serial.println(buf);
    snprintf(buf, sizeof(buf), "low 0x%x", (unsigned char)(eeaddress & 0xff));
    Serial.println(buf);

    Wire.beginTransmission(m_id);
    Wire.write((unsigned char)(eeaddress >> 8));   // MSB
    Wire.write((unsigned char)(eeaddress & 0xff)); // LSB
    Wire.endTransmission();

        snprintf(buf, sizeof(buf), "len %u", data_len);
        Serial.println(buf);
    Wire.requestFrom(m_id, data_len);

    delay(100);
        snprintf(buf, sizeof(buf), "avail %u", Wire.available());
        Serial.println(buf);
    unsigned char i = 0;
    while(Wire.available())
    {
        data[i++] = Wire.read();
        snprintf(buf, sizeof(buf), "rb 0x%x", data[i-1]);
        Serial.println(buf);
    }

    snprintf(buf, sizeof(buf), "fin 0x%x", i);
    Serial.println(buf);
}

i2c_list::i2c_list(id_type id) :
    i2c_eeprom(id)
{
}

void i2c_list::format()
{
    //{
        char data[BLOCK_SIZE];
        memset(data, 1, BLOCK_SIZE);
        // Start of the data list (initially not created).
        //address start(data);
        //start = 0;
        // Start of the free file.
        //address start_free(data + 2);
        //start_free = 0;
        write(0, data, BLOCK_SIZE);
    //}
    //for(int i = 1; i < block_count(); ++i)
    //{
        //block_type data;
        //memset(data, 0, BLOCK_SIZE);
        //// Next block.
        //address next(data);
        //next = (i == block_count()) ? 0 : i + 1;
        //// Previous block.
        //address prev(data + 2);
        //prev = (i == 1) ? 0 : i - 1;
        //write(i * BLOCK_SIZE, data, BLOCK_SIZE);
    //}
}

void i2c_list::append(const char *li)
{
    block_type block;
    address_type new_block = NULLPTR;
    if((new_block = alloc_back(block)) == NULLPTR)
        return;
    memcpy(block + 4, li, MAX(strlen(li), BLOCK_SIZE - 4));
    write(new_block * BLOCK_SIZE, block, BLOCK_SIZE);
}

void i2c_list::prepend(const char *li)
{
    block_type block;
    address_type new_block = NULLPTR;
    if((new_block = alloc_front(block)) == NULLPTR)
        return;
    memcpy(block + 4, li, MAX(strlen(li), BLOCK_SIZE - 4));
    write(new_block * BLOCK_SIZE, block, BLOCK_SIZE);
}

bool i2c_list::first(block_type& out)
{
    block_type superblock;
    read(0, superblock, BLOCK_SIZE);
    char buf[16];
    snprintf(buf, sizeof(buf), "c0 %u", (unsigned char)superblock[0]);
    Serial.println(buf);
    snprintf(buf, sizeof(buf), "c1 %u", (unsigned char)superblock[1]);
    Serial.println(buf);
    snprintf(buf, sizeof(buf), "block 0x%hx", (address_type)address(superblock));
    Serial.println(buf);
    if((address_type)address(superblock) == NULLPTR)
        return false;
    read((address_type)address(superblock) * BLOCK_SIZE, out, BLOCK_SIZE);
    return true;
}

i2c_eeprom::address_type i2c_list::free_addr() const
{
    block_type block;
    // Read the superblock.
    read(0, block, BLOCK_SIZE);
    // Get the address of the start of the free list from the superblock.
    return address(block + 2);
}

i2c_eeprom::address_type i2c_list::start_addr() const
{
    char data[BLOCK_SIZE];
    // Read the superblock.
    read(0, data, BLOCK_SIZE);
    // Get the address of the start of the file from the superblock.
    return address(data);
}

i2c_eeprom::address_type i2c_list::remove(block_type& block)
{
    address_type prev_addr = address(block);
    address_type next_addr = address(block + 2);
    if(prev_addr != NULLPTR)
    {
        block_type prev;
        read(prev_addr * BLOCK_SIZE, prev, BLOCK_SIZE);
        address prev_next(prev + 2);
        prev_next = next_addr;
        write(prev_addr * BLOCK_SIZE, prev, BLOCK_SIZE);
    }
    if(next_addr != NULLPTR)
    {
        block_type next;
        read(next_addr * BLOCK_SIZE, next, BLOCK_SIZE);
        address next_prev(next);
        next_prev = next_addr;
        write(next_addr * BLOCK_SIZE, next, BLOCK_SIZE);
    }
    return (prev_addr == NULLPTR) ? next_addr : prev_addr;
}

i2c_eeprom::address_type i2c_list::alloc_front(block_type& out)
{
    block_type superblock;
    read(0, superblock, BLOCK_SIZE);

    if((address_type)address(superblock + 2) == NULLPTR)
        return NULLPTR;

    block_type first_free;
    read((address_type)address(superblock + 2) * BLOCK_SIZE, first_free, BLOCK_SIZE);
    address_type new_first_free_addr = remove(first_free);
    write((address_type)address(superblock + 2) * BLOCK_SIZE, first_free, BLOCK_SIZE);
    address(superblock + 2) = new_first_free_addr;

    if((address_type)address(superblock) == NULLPTR)
    {
        // This is a new file.
        address addr(superblock);
        addr = new_first_free_addr;
    }
    else
    {
        // This is not a new file.
        block_type next;
        read((address_type)address(superblock) * BLOCK_SIZE, next, BLOCK_SIZE);
        address next_addr(next);
        next_addr = new_first_free_addr;
        write((address_type)address(superblock) * BLOCK_SIZE, next, BLOCK_SIZE);
        address addr(superblock);
        addr = new_first_free_addr;
    }

    write(0, superblock, BLOCK_SIZE);
}

i2c_eeprom::address_type i2c_list::alloc_back(block_type& out)
{
}

i2c_map::i2c_map(id_type id) :
    i2c_list(id)
{
}

