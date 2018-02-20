/* --std=c90 --input-charset=euckr --exec-charset=euckr --wide-exec-charset=ucs4 -Wv */

char a[] = "1234567891123456789212345678931234567894123456789512345678961234567897123456789812345678991234567890"
           "1234567891123456789212345678931234567894123456789512345678961234567897123456789812345678991234567890"
           "1234567891123456789212345678931234567894123456789512345678961234567897123456789812345678991234567890"
           "1234567891123456789212345678931234567894123456789512345678961234567897123456789812345678991234567890"
           "1234567891123456789212345678931234567894123456789512345678961234567897123456789812345678991234567890"
           "123456789";
char b[] = "1234567891123456789212345678931234567894123456789512345678961234567897123456789812345678991234567890"
           "1234567891123456789212345678931234567894123456789512345678961234567897123456789812345678991234567890"
           "1234567891123456789212345678931234567894123456789512345678961234567897123456789812345678991234567890"
           "1234567891123456789212345678931234567894123456789512345678961234567897123456789812345678991234567890"
           "1234567891123456789212345678931234567894123456789512345678961234567897123456789812345678991234567890"
           "1234567891";
char c[] = "���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           "���̻�����ĥ�ȱ������̻�����ĥ�ȱ�ĥ���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           "���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           "���̻�����ĥ�ȱ������̻�����ĥ�ȱ�";    /* 169 * 3 = 507 */
char d[] = "���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           "���̻�����ĥ�ȱ������̻�����ĥ�ȱ�ĥ���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           "���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           "���̻�����ĥ�ȱ������̻�����ĥ�ȱ�ĥ";    /* 170 * 3 = 510 */
long e[] = "���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           "���̻�����ĥ�ȱ������̻�����ĥ�ȱ�ĥ���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           "���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           "���̻�����ĥ�ȱ������̻�����ĥ�ȱ�ĥ���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           "���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           "���̻�����ĥ�ȱ������̻�����ĥ�ȱ�ĥ���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           "���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           "���̻�����ĥ�ȱ������̻�����ĥ�ȱ�ĥ���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           "���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           "���̻�����ĥ�ȱ������̻�����ĥ�ȱ�ĥ���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           "���̻�����ĥ�ȱ�";
long f[] = "���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           "���̻�����ĥ�ȱ������̻�����ĥ�ȱ�ĥ���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           "���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           "���̻�����ĥ�ȱ������̻�����ĥ�ȱ�ĥ���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           "���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           "���̻�����ĥ�ȱ������̻�����ĥ�ȱ�ĥ���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           "���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           "���̻�����ĥ�ȱ������̻�����ĥ�ȱ�ĥ���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           "���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           "���̻�����ĥ�ȱ������̻�����ĥ�ȱ�ĥ���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           "���̻�����ĥ�ȱ���";
long g[] =  "���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           L"���̻�����ĥ�ȱ������̻�����ĥ�ȱ�ĥ���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           L"���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           L"���̻�����ĥ�ȱ������̻�����ĥ�ȱ�ĥ���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           L"���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           L"���̻�����ĥ�ȱ������̻�����ĥ�ȱ�ĥ���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
            "���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           L"���̻�����ĥ�ȱ������̻�����ĥ�ȱ�ĥ���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           L"���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           L"���̻�����ĥ�ȱ������̻�����ĥ�ȱ�ĥ���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           L"���̻�����ĥ�ȱ�";
long h[] = L"���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           L"���̻�����ĥ�ȱ������̻�����ĥ�ȱ�ĥ���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           L"���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
            "���̻�����ĥ�ȱ������̻�����ĥ�ȱ�ĥ���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           L"���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           L"���̻�����ĥ�ȱ������̻�����ĥ�ȱ�ĥ���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           L"���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           L"���̻�����ĥ�ȱ������̻�����ĥ�ȱ�ĥ���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           L"���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           L"���̻�����ĥ�ȱ������̻�����ĥ�ȱ�ĥ���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
            "���̻�����ĥ�ȱ���";
char i[] = "12345��ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           "���̻�����ĥ�ȱ������̻�����ĥ�ȱ�ĥ���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           "���̻�����ĥ�ȱ������̻��\xaa\xbb\xcc\xdd�������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           "���̻�����ĥ�ȱ������̻�����ĥ�ȱ�ĥ���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           "���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           "���̻�����ĥ�ȱ������̻�����ĥ�ȱ�ĥ123456789�����̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           "���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           "���̻�����ĥ�ȱ������̻�����ĥ�ȱ�ĥ���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           "���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           "���̻�����ĥ�ȱ������̻��\005\006ĥ�ȱ�ĥ���̻�\004����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           "���̻�����ĥ�ȱ�";
char j[] = "12345��ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           "���̻�����ĥ�ȱ������̻�����ĥ�ȱ�ĥ���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           "���̻�����ĥ�ȱ������̻��\xaa\xbb\xcc\xdd�������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           "���̻�����ĥ�ȱ������̻�����ĥ�ȱ�ĥ���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           "���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           "���̻�����ĥ�ȱ������̻�����ĥ�ȱ�ĥ123456789�����̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           "���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           "���̻�����ĥ�ȱ������̻�����ĥ�ȱ�ĥ���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           "���̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           "���̻�����ĥ�ȱ������̻��\005\006ĥ�ȱ�ĥ���̻�\004����ĥ�ȱ������̻�����ĥ�ȱ������̻�����ĥ�ȱ���"
           "���̻�����ĥ�ȱ���";
