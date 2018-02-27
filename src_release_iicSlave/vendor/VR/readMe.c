/* \iic slave demon 指南
 *
 * \version 01.10
 *
 * \date 2018-02-28
 *
 * - GP4 GP5 模拟master iic 对从iic（GP22 gp23） 进行读写操作
 * - GP18 为唤醒io口
 * - master发送数据给APP，master iic 读写之前需要对唤醒io进行拉高操作，使mcu不进入睡眠，然后写入数据。slave buffer的前64个byte用于接收写入的数据。
 *   处理接收到的数据然后发送给app
 * - app下发数据给mcu，mcu把接收到的数据存储在slave buffer的后 64byte ，然后通过 GP18 拉高，告知 master 去读取iic数据。
 *   master 需要在mcu不休眠的情况下完成读取数据操作
 */

