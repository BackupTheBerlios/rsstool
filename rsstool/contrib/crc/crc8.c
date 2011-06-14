#if 0
uint8 Crc8_u8CalculateCRC(const uint8 *p, uint n)
{
uint8 u8checksum = 0;
uint i;

for (i=0;i<n;i++)
{
u8checksum = crc_table[u8checksum ^ *p++];
}

return u8checksum;
}

/**/

Define_Function INTEGER DoCRC8 (CHAR stringIn[30])
{
CHAR tmpStr[30]
INTEGER i
INTEGER n
INTEGER m

crc8Sum = 0
tmpStr = stringIn
while (Length_String (tmpStr))
{
i = Get_Buffer_Char (tmpStr)
n = crc8Sum BXOR i
crc8Sum = crc8Table[n]
}
RETURN crc8Sum
}
#endif
#include <stdio.h>


#define CRCRES 0xf0b8 /* residue for good verify */
#define DEBUG

unsigned crctbl[] = {0x0000, 0x1081, 0x2102, 0x3183,
0x4204, 0x5285, 0x6306, 0x7387,
0x8408, 0x9489, 0xa50a, 0xb58b,
0xc60c, 0xd68d, 0xe70e, 0xf78f};

/*
* This uses a 32 byte table to lookup the crc 4 bits at a time.
* The CRC CCITT is used.
*/


unsigned short calc_crc(unsigned char *ptr, unsigned length)

{
unsigned short crc;
unsigned short i;
unsigned char pos,ch;

crc = 0xffff; /* precondition crc */
for (i = 0; i < length; i++,ptr++) {
ch = *ptr;
pos = (crc ^ ch) & 15;
crc = ((crc >> 4) & 0x0fff) ^ crctbl[pos];
ch >>= 4;
pos = (crc^ch) & 15;
crc = ((crc >> 4) & 0xffff) ^ crctbl[pos];
}
crc = ~crc; /* post condition */
crc = (crc << 8) | (crc >> 8); /* bytewise reverse */
return crc;
}


#define POLYNOMIAL    (0x1070U << 3) 

unsigned char misc_crc8 ( unsigned char inCrc, unsigned char inData )
{
    int i;
    unsigned short  data;

    data = inCrc ^ inData;
    data <<= 8;
  
    for ( i = 0; i < 8; i++ ) 
    {
        if (( data & 0x8000 ) != 0 )
        {
            data = data ^ POLYNOMIAL;
        }
        data = data << 1;
    }

  // DEBUG
//  printf ( "Crc8: data:0x%02x crc:0x%02x\n", inData, (unsigned char)( data >> 8 ));

    return (unsigned char)( data >> 8 );

} // Crc8



int main ()

{
unsigned short crc = 0;
unsigned char arr [] = {'X','Y','Z'};

crc = misc_crc8(crc, 't');
crc = misc_crc8(crc, 'e');
crc = misc_crc8(crc, 's');
crc = misc_crc8(crc, 't');
printf("Calced crc of test to be %x\n",crc);
}

