#ifndef AETHER_SENSOR_EEPROM_HPP
#define AETHER_SENSOR_EEPROM_HPP

class i2c_eeprom
{
public:
    /*!
     * \brief Type of an I2C device id.
     */
    typedef int id_type;
    /*!
     * \brief Type of an address within an I2C EEPROM.
     */
    typedef unsigned int address_type;

    /*!
     * \param i2c_id I2C address of the EEPROM.
     */
    i2c_eeprom(id_type i2c_id);

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
    void write(address_type eeaddress, char *data, unsigned int data_len);
    /*!
     * \brief Read raw bytes from the EEPROM.
     *
     * \note The EEPROM storage is organised into 64-byte pages.  Reads that do
     * not cover a full page will be slightly slower, as whole pages must be
     * read and unwanted parts discarded.
     */
    void read(address_type eeaddress, char *data, unsigned int data_len);

private:
    int m_id;
};

class i2c_map :
    public i2c_eeprom
{
public:
    i2c_map(id_type id);
};

#endif

