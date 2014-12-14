#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/module.h>



MODULE_LICENSE( "GPL" );
MODULE_AUTHOR( "Taku Gunji" );

#define CHIP_ADDRESS 0x40 /* i2c address. */
#define BUS_NO 0 /* i2c bus. */

/* Internal representation of one chip. */
struct sht25_data {
  s16 temp;
  s16 rh;
  struct mutex lock;
  unsigned long timestamp;
};

/*                                                                                                                                                
 * I2C.                                                                                                                                           
 *                                                                                                                                                
 * Code for accesing the chip over i2c protocoll.                                                                                                 
 */
#define TEMP_READ 0xe3
#define RH_READ 0xe5
#define TEMP_REG 0x03
#define RH_REG 0x05

static inline int
sht25_init_chip( struct i2c_client *client )
{

  return 0;
}

struct sht25_data *sht25_update_chip( struct i2c_client *client )
{
  u8 buff[2];
  struct sht25_data *data;
  //int channel;

  data = i2c_get_clientdata( client );
  mutex_lock( &data->lock );

  if ( !time_after( jiffies, data->timestamp ) ) {
    mutex_unlock( &data->lock );
    return data;
  }
  /* Read temperature. */
  buff[0] = TEMP_READ;
  i2c_master_send( client, buff, 1 );
  buff[0] = TEMP_REG;
  i2c_master_send( client, buff, 1 );
  i2c_master_recv( client, buff, 2 );
  data->temp = (buff[0]<<8) | buff[1];
  //data->temp =  -46.85 + 175.72*data->temp/65536;

  /* Read RH. */
  buff[0] = RH_READ;
  i2c_master_send( client, buff, 1 );
  buff[0] = RH_REG;
  i2c_master_send( client, buff, 1 );
  i2c_master_recv( client, buff, 2 );
  data->rh = (buff[0]<<8) | buff[1];


  mutex_unlock( &data->lock );


  return data;
}


/*                                                                                                                                                
 * SYSFS.                                                                                                                                         
 *                                                                                                                                                
 * Code for creating entries in sysfs.                                                                                                            
 */
static ssize_t sht25_show_temp( struct device *dev, struct device_attribute *attr, char *buff )
{
  struct sht25_data *data;
  struct i2c_client *client;

  client = to_i2c_client( dev );
  data = sht25_update_chip( client );
  return snprintf( buff, PAGE_SIZE, "%d\n", data->temp );
}

static ssize_t sht25_show_rh( struct device *dev, struct device_attribute *attr, char *buff )
{
  struct sht25_data *data;
  struct i2c_client *client;

  client = to_i2c_client( dev );
  data = sht25_update_chip( client );
  return snprintf( buff, PAGE_SIZE, "%d\n", data->rh );
}



static SENSOR_DEVICE_ATTR( temperature, S_IRUGO, sht25_show_temp, NULL, 0 );
static SENSOR_DEVICE_ATTR( humidity, S_IRUGO, sht25_show_rh, NULL, 1 );

static struct attribute *sht25_attributes[] = {
  &sensor_dev_attr_temperature.dev_attr.attr,
  &sensor_dev_attr_humidity.dev_attr.attr,
  NULL,
};

static const struct attribute_group sht25_grp = {
  .attrs = sht25_attributes,
};

/*                                                                                                                                                
 * Kernel module.                                                                                                                                 
 *                                                                                                                                                
 * Code for initializing the kernel module.                                                                                                       
 */

static const struct i2c_device_id sht25_id[] = {
  { "sht25", 0 },
  { }
};

MODULE_DEVICE_TABLE( i2c, sht25_id );

static int sht25_probe( struct i2c_client *client, const struct i2c_device_id *id )
{
  int err;
  struct sht25_data *data;
  //dev_info( &client->dev, "Probing for %s\n", client->name );                                                                                   

  err = sht25_init_chip( client );
  if ( err != 0 ) {
    return -ENODEV;
  }

  data = kzalloc( sizeof( struct sht25_data ), GFP_KERNEL );
  if ( !data )
    return -ENOMEM;

  mutex_init( &data->lock );
  data->timestamp = jiffies;
  i2c_set_clientdata( client, data );

  err = sysfs_create_group( &client->dev.kobj, &sht25_grp );
  if ( err ) {
    goto err_free;
    dev_err( &client->dev, "Could not create sysfs group. Errno: %d\n",
             -err );
  }

  return 0;

 err_free:
  kfree( data );
  return err;
}

static int sht25_remove( struct i2c_client *client )
{
  sysfs_remove_group( &client->dev.kobj, &sht25_grp );
  kfree( i2c_get_clientdata( client ) );
  return 0;
}

static struct i2c_driver sht25_driver = {
  .driver = {
    .name = "sht25",
  },
  .probe = sht25_probe,
  .remove = sht25_remove,
  .id_table = sht25_id,
};

static struct i2c_board_info adc_board_info = {
  I2C_BOARD_INFO( "sht25", CHIP_ADDRESS ),
};

static struct i2c_client *cli_handle;

static int __init
sht25_init(void)
{
  int ret;
  
  ret = i2c_add_driver( &sht25_driver );
  if ( ret < 0 )
    return ret;

  /* Probe for a chip at the specified address. */
  cli_handle = i2c_new_device( i2c_get_adapter(BUS_NO), &adc_board_info );

  return 0;
}

static void __exit
sht25_exit(void)
{
  i2c_unregister_device( cli_handle );
  i2c_del_driver( &sht25_driver );
}

module_init( sht25_init );
module_exit( sht25_exit );


