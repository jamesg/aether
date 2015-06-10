#ifndef AETHER_SENSOR_EEPROM_HPP
#define AETHER_SENSOR_EEPROM_HPP

/*!
 * \brief Read and write Microchip I2C EEPROM devices.
 */
class i2c_eeprom
{
public:
    /*!
     * \brief Type of an I2C device id.
     */
    typedef int id_type;
    /*!
     * \brief Type of a block address within an I2C EEPROM.
     */
    typedef short unsigned int address_type;

    /*!
     * \brief Conversion type for addresses.
     */
    class address
    {
    public:
        address(char *start);
        address& operator=(address_type);
        operator address_type() const;
    private:
        char *m_start;
    };

    static const address_type BLOCK_COUNT = 2;
    static const address_type BLOCK_SIZE = 8;
    static const address_type NULLPTR = 0;

    /*!
     * \param i2c_id I2C address of the EEPROM.
     */
    i2c_eeprom(id_type i2c_id);

    /*!
     * \brief Get the number of blocks on the EEPROM.
     */
    address_type block_count() const;

protected:
    /*!
     * \brief Write raw bytes to the EEPROM.
     *
     * \note The EEPROM storage is organised into 64-byte pages.  Writes that
     * do not cover a full page are supported, but the EEPROM will still have
     * to overwrite the entire page, potentially increasing wear.  If many
     * small writes are to be made to the same page, it is best to cache the
     * writes first, then call this function.
     */
    void write(unsigned int eeaddress, char *data, unsigned int data_len);
    /*!
     * \brief Read raw bytes from the EEPROM.
     *
     * \note The EEPROM storage is organised into 64-byte pages.  Reads that do
     * not cover a full page will be slightly slower, as whole pages must be
     * read and unwanted parts discarded.
     */
    void read(unsigned int eeaddress, char *data, unsigned int data_len) const;

private:
    int m_id;
};

/*!
 * \brief A linked list stored on an I2C EEPROM.
 */
class i2c_list :
    public i2c_eeprom
{
public:
    typedef char block_type[BLOCK_SIZE];

    i2c_list(id_type id);

    /*!
     * \brief Erase all data on the device and set up a new list.
     */
    void format();

    /*!
     * \brief Append a list item to the end of the file.
     */
    void append(const char*);

    /*!
     * \brief Prepend a list item to the start of the file.
     */
    void prepend(const char*);

    bool first(block_type& out);

protected:
    /*!
     * \brief Get the address of the first free block.
     */
    address_type free_addr() const;
    /*!
     * \brief Get the address of the start of the list.
     */
    address_type start_addr() const;

    /*!
     * \brief Remove a block from a file.
     *
     * \note The block will not be added to the free file.
     *
     * \return The address of the previous block, or the address of the next
     * block if there was no previous block.  If this was the first block in
     * the file, pointers to the file should be updated to the returned
     * address.
     */
    address_type remove(block_type&);

    /*!
     * \brief Allocate a block at the start of the file.
     *
     * \return The block number allocated.
     */
    address_type alloc_front(block_type&);

    /*!
     * \brief Allocate a block at the end of the file.
     *
     * \return The block number allocated.
     */
    address_type alloc_back(block_type&);
};

/*!
 * \brief A key-value map stored on an I2C EEPROM.
 */
class i2c_map :
    public i2c_list
{
public:
    i2c_map(id_type id);
};

#endif

