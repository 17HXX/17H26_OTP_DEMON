/* \iic slave demon ָ��
 *
 * \version 01.10
 *
 * \date 2018-02-28
 *
 * - GP4 GP5 ģ��master iic �Դ�iic��GP22 gp23�� ���ж�д����
 * - GP18 Ϊ����io��
 * - master�������ݸ�APP��master iic ��д֮ǰ��Ҫ�Ի���io�������߲�����ʹmcu������˯�ߣ�Ȼ��д�����ݡ�slave buffer��ǰ64��byte���ڽ���д������ݡ�
 *   ������յ�������Ȼ���͸�app
 * - app�·����ݸ�mcu��mcu�ѽ��յ������ݴ洢��slave buffer�ĺ� 64byte ��Ȼ��ͨ�� GP18 ���ߣ���֪ master ȥ��ȡiic���ݡ�
 *   master ��Ҫ��mcu�����ߵ��������ɶ�ȡ���ݲ���
 */

