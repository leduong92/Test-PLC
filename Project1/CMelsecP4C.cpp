#include "CMelsecP4C.h"

CMelsecP4C::CMelsecP4C()
{
	m_fOpen = FALSE;
}


CMelsecP4C::~CMelsecP4C()
{
	TRS232C::SetRTS(FALSE);
	TRS232C::SetDTR(FALSE);
	TRS232C::Close();
}
void CMelsecP4C::ComClose(void)
{
	if (m_fOpen) {
		TRS232C::SetRTS(FALSE);
		TRS232C::SetDTR(FALSE);
		TRS232C::Close();
		m_fOpen = FALSE;
	}
}

BOOL CMelsecP4C::Init(int portNo, int baudrate, int databit, int stopbit, int parity, int wait, BOOL ctrl_line)
{
	if (m_fOpen)
		return TRUE;

	m_wait = wait;

	PORTSETTING ps;
	memset(&ps, 0, sizeof(PORTSETTING));

	switch (baudrate) {
	case 300:
		ps.BauRate = CBR_300;				//1200bps
		break;
	case 600:
		ps.BauRate = CBR_600;				//1200bps
		break;
	case 1200:
		ps.BauRate = CBR_1200;				//1200bps
		break;
	case 2400:
		ps.BauRate = CBR_2400;				//bps
		break;
	case 4800:
		ps.BauRate = CBR_4800;				//bps
		break;
	case 14400:
		ps.BauRate = CBR_14400;				//bps
		break;
	case 19200:
		ps.BauRate = CBR_19200;				//bps
		break;
	case 28800:
		ps.BauRate = 28800;				//bps
		break;
	case 38400:
		ps.BauRate = CBR_38400;				//bps
		break;
	case 57600:
		ps.BauRate = CBR_57600;				//bps
		break;
	case 115200:
		ps.BauRate = CBR_115200;				//bps
		break;
	case 9600:
	default:
		ps.BauRate = CBR_9600;				//1200bps
		break;
	}

	// DataBit
	switch (databit) {
	case 7:
		ps.DByte = 7;
		break;
	case 8:
	default:
		ps.DByte = 8;
		break;
	}

	// STOP BIT
	switch (stopbit) {
	case 1:
		ps.StopBits = ONESTOPBIT;
		break;
	case 2:
	default:
		ps.StopBits = TWOSTOPBITS;
		break;
	}

	// PARITY
	switch (parity) {
	case 1:
		ps.Parity = ODDPARITY;
		break;
	case 2:
		ps.Parity = EVENPARITY;
		break;
	case 0:
	default:
		ps.Parity = NOPARITY;
		break;
	}

	ps.fBinary = TRUE;

	if (ctrl_line == TRUE) {
		ps.fOutxCtsFlow = TRUE;
		ps.fOutxDsrFlow = TRUE;
		ps.fDtrControl = DTR_CONTROL_HANDSHAKE;
		ps.fRtsControl = RTS_CONTROL_HANDSHAKE;

	}
	else {
		ps.fOutxCtsFlow = FALSE;		// TRUE or FALSE
		ps.fOutxDsrFlow = FALSE;		// TRUE or FALSE
		ps.fDtrControl = DTR_CONTROL_DISABLE;
		ps.fRtsControl = RTS_CONTROL_DISABLE;
	}

	if (!TRS232C::CommOpen((long)portNo, &ps)) {
		return (FALSE);
	}
	if (ctrl_line == TRUE) {
		TRS232C::SetRTS(TRUE);
		TRS232C::SetDTR(TRUE);
	}
	TRS232C::ClearRcvBuffer();
	m_fOpen = TRUE;

	return TRUE;
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>READ/WRITE BIT>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int CMelsecP4C::P4WriteB(char * station, char * pcnumber, char * address, int onoff)
{
	int ret, len;
	int retry = 0;
	char data[20];
	char send[10];


	if (m_fOpen == FALSE)
		return -1;

	if (!onoff)
		strcpy(send, "0\0");
	else
		strcpy(send, "1\0");

	MakeCmd(m_buf, sizeof(m_buf), station, pcnumber, "BW", address, 1, send);

	while (retry < 8)
	{
		TRS232C::ClearRcvBuffer();

		ret = TRS232C::Write((BYTE*)m_buf, strlen(m_buf));

		//Receive 1 byte and check ACK or NAK
		ret = Receive(data, sizeof(data), 1);
		if (ret)
			return (-3);

		if (data[0] == ACK)
			len = 6;
		else if (data[0] == NAK)
			len = 8;
		else {
			if (retry++ < 8)continue;
			return(-4);
		}
			

		ret = Receive(data, sizeof(data), len);

		if (ret)
			return (-5);

		if (len == 8)
			retry++;
		else
			break;

	}
	if (retry >= 8)
		return (-9);

	return 0;
}
int CMelsecP4C::P4ReadB(char * station, char * pcnumber, char * address)
{
	int ret, len;
	int retry = 0, sum;
	char data[20], txt[10];
	char buf;

	if (m_fOpen == FALSE) return (-1);

	memset(data, 0, sizeof(data));

	while (retry < 8)
	{
		MakeCmd(m_buf, sizeof(m_buf), station, pcnumber, "BR", address, 8, "");

		TRS232C::ClearRcvBuffer();
		ret = TRS232C::Write((BYTE*)m_buf, strlen(m_buf));

		ret = Receive(data, sizeof(data), 1);
		if (ret)
			return (-3);
		if (data[0] == STX) { //Determine the remaining number of processes for each command)
			len = 0;
			memset(m_buf, 0, sizeof(m_buf));
			while (data[0] == ETX)
			{
				ret = Receive(data, sizeof(data), 1);
				if (ret)
					return (-4);
				m_buf[len] = data[0];
				if (++len == sizeof(m_buf))
					return (-5);
			}
			ret = Receive(data, sizeof(data), 4);
			if (ret)
				return (-6);

			sum = GetSumCheckCode(m_buf);
			wsprintf(txt, "%02X", sum);

			if (!memcmp(txt, data, 2)) {
				Answer(ACK, station, pcnumber);
				break;
			}

			Answer(NAK, station, pcnumber);
			retry++;
			continue;
		}
		else if (data[0] == NAK) {
			ret = Receive(data, sizeof(data), 8);
			if (ret)
				return (-7);
			retry++;
			continue;
		}
		else return (-8);

	}
	if (retry >= 8)
		return (-9);

	buf = m_buf[4]; //Copy received data to buffer

	return (buf - 0x30);
}
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<READ/WRITE BIT<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>READ/WRITE WORD>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int CMelsecP4C::P4WriteW(char * station, char * pcstation, char * address, short counts, char * msg)
{
	int ret, len;
	int retry = 0;
	char data[20];
	char initcom[5];

	initcom[0] = 4;
	initcom[1] = 0xD;
	initcom[2] = 0xA;
	initcom[3] = 0;

	if (m_fOpen == FALSE) return (-1);
	if (counts < 0 || counts > 64) return (-2);

	if (strlen(address) + strlen(msg) + P4_HEADLEN1 >= sizeof(m_buf)) return (-10);
	//number of device points * 4 character
	if (counts * 4 != (int)strlen(msg)) return (-11);

	if (strlen(address) == 5)
		MakeCmd(m_buf, sizeof(m_buf), station, pcstation, "WW", address, counts, msg);
	else
		MakeCmd(m_buf, sizeof(m_buf), station, pcstation, "QW", address, counts, msg);

	while (retry < 8)
	{
		if (retry) {
			TRS232C::ClearRcvBuffer();
			ret = TRS232C::Write((BYTE*)initcom, 3);
		}
		TRS232C::ClearRcvBuffer();
		ret = TRS232C::Write((BYTE*)m_buf, strlen(m_buf));
		if (ret != strlen(m_buf)) return(-3);

		ret = Receive(data, sizeof(data), 1);
		if (ret) {
			if (retry++ < 8)
				continue;
			return(-3);
		}

		if (data[0] == ACK)
			len = 6;
		else if (data[0] == NAK)
			len = 8;
		else {
			if (retry++ < 8)
				continue;
			return (-5);
		}

		if (len == 8)
			retry++;
		else
			break;
	}
	if (retry >= 8)
		return (-9);

	return 0;
}

int CMelsecP4C::P4ReadW(char * buf, int bufsize, char * station, char * pcstation, char * addr, short counts)
{
	int ret, len;
	int retry = 0, sum;
	char data[20], txt[10];
	char initcom[5];

	initcom[0] = 4;
	initcom[1] = 0xD;
	initcom[2] = 0xA;
	initcom[3] = 0;

	if (m_fOpen == FALSE) return(-1);
	if (counts < 0 || counts > 64) return(-2);

	memset(data, 0, sizeof(data));

	while (retry < 8)
	{
		if (retry) {
			TRS232C::ClearRcvBuffer();
			ret = TRS232C::Write((BYTE*)initcom, 3);
		}

		if (strlen(addr) == 5)
			MakeCmd(m_buf, sizeof(m_buf), station, pcstation, "WR", addr, counts, "");
		else
			MakeCmd(m_buf, sizeof(m_buf), station, pcstation, "QR", addr, counts, "");

		TRS232C::ClearRcvBuffer();
		ret = TRS232C::Write((BYTE*)m_buf, strlen(m_buf));

		ret = Receive(data, sizeof(data), 1);
		if (ret) return(-3);

		if (data[0] == STX)
		{
			len = 0;
			memset(m_buf, 0, sizeof(m_buf));
			while (data[0] != ETX) {
				ret = Receive(data, sizeof(data), 1);
				if (ret) {
					return(-4);
				}
				m_buf[len] = data[0];
				if (++len == sizeof(m_buf)) return(-5);
			}

			ret = Receive(data, sizeof(data), 4);
			if (ret) return(-6);

			sum = GetSumCheckCode(m_buf);
			wsprintf(txt, "%02X", sum);

			if (!memcmp(txt, data, 2)) {
				Answer(ACK, station, pcstation);
				break;
			}
			Answer(NAK, station, pcstation);
			retry++;
			continue;
		}
		else if (data[0] == NAK) {
			ret = Receive(data, sizeof(data), 8);
			if (ret) return(-7);
			retry++;
			continue;
		}
		else return(-8);
	}

	if (retry >= 8) return(-9);

	memcpy(buf, m_buf + 4, (int)(strlen(m_buf + 4) - 1) > bufsize ? bufsize : strlen(m_buf + 4) - 1);
	return 0;
}

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<READ/WRITE WORD<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

//==========================================HELPER============================================
void CMelsecP4C::MakeCmd(char * buf, int bufsize, char * station, char * pc, char * cmd, char * address, short counts, char * msg)
{
	char txt[10];
	int sum;

	memset(buf, 0, bufsize);
	buf[0] = ENQ;

	strcat(buf, station);
	strcat(buf, pc);

	strcat(buf, cmd);

	wsprintf(txt, "%d", m_wait);
	memcpy(&buf[strlen(buf)], txt, 1);

	strcat(buf, address);

	wsprintf(txt, "%02X", counts); //number of device block -- max 64
	memcpy(&buf[strlen(buf)], txt, 2);

	strcat(buf, msg);

	sum = GetSumCheckCode(&buf[1]);
	wsprintf(txt, "%02X", sum);
	memcpy(&buf[strlen(buf)], txt, 2);

	buf[strlen(buf)] = CR;
	buf[strlen(buf)] = LF;

}



int CMelsecP4C::GetSumCheckCode(char * str)
{
	int data = 0;
	int i, len;

	if (str == NULL)
		return (0);

	len = strlen(str);
	if (len <= 0)
		return (0);
	for (int i = 0; i < len; i++)
	{
		data += (int)str[i];
	}

	data = data & 0x000000ff;

	return data;
}


// int CMelsecP4Ex :: Receive (char * buf, int bufsize, int waitlen)
// Function: Read data from receive buffer
// type: private
// Argument 1: Buffer
// Argument 2: Buffer size
// Argument 3: Number of data bytes waiting to be received
int CMelsecP4C::Receive(char * buf, int bufsize, int waitlen)
{
	int theLen, prevLen = 0;
	int counter = 0;
	BOOL bReadStatus;

	int status;

	if (m_fOpen == FALSE)
		return (-1);
	if (waitlen < 1)
		return (-2);
	if (bufsize < waitlen)
		return (-2);

	memset(buf, 0, sizeof(bufsize));

	while (1)
	{

		theLen = TRS232C::ReadLenght();  //Check if there is data in the receive buffer

		if (theLen >= waitlen) //The specified number of bytes has been reached
		{
			break;
		}
		else { // less than the specified number of bytes
			Sleep(15);
			if (theLen != prevLen) {
				theLen = prevLen;
				counter = 0;
			}
			else {
				counter++; //Number of bytes received does not change
			}
		}
		//Time out
		if (counter > 40) {
			return (-3);
		}
	}
	TRS232C::Read((BYTE*)buf, waitlen);

	return 0;
}

int CMelsecP4C::Answer(char flg, char* station, char* pc)
{
	char buf[10];
	int ret;

	memset(buf, 0, sizeof(buf));

	buf[0] = flg;
	memcpy(buf + 1, station, 2);
	memcpy(buf + 3, pc, 2);
	buf[5] = CR;
	buf[6] = LF;

	ret = TRS232C::Write((BYTE*)buf, strlen(buf));
	return 0;
}

//==========================================HELPER============================================

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>READ/WRITE Multiple Point Bit>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int CMelsecP4C::P4ReadBB(char * buf, int bufsize, char * station, char * pcstation, char * addr, short counts)
{
	int ret, len;
	int retry = 0, sum;
	char data[20], txt[10];

	if (m_fOpen == FALSE) return(-1);
	memset(data, 0, sizeof(data));

	while (retry < 8) {
		MakeCmd(m_buf, sizeof(m_buf), station, pcstation, "BR", addr, counts, "");

		TRS232C::ClearRcvBuffer();
		ret = TRS232C::Write((BYTE*)m_buf, strlen(m_buf));

		ret = Receive(data, sizeof(data), 1);
		if (ret) return(-3);

		if (data[0] == STX) {
			len = 0;
			memset(m_buf, 0, sizeof(m_buf));
			while (data[0] != ETX) {
				ret = Receive(data, sizeof(data), 1);
				if (ret) return(-4);
				m_buf[len] = data[0];
				if (++len == sizeof(m_buf)) return(-5);
			}

			ret = Receive(data, sizeof(data), 4);
			if (ret) return(-6);

			sum = GetSumCheckCode(m_buf);
			wsprintf(txt, "%02X", sum);

			if (!memcmp(txt, data, 2)) {
				Answer(ACK, station, pcstation);
				break;
			}
			Answer(NAK, station, pcstation);
			retry++;
			continue;
		}
		else if (data[0] == NAK) {
			ret = Receive(data, sizeof(data), 8);
			if (ret) return(-7);
			retry++;
			continue;
		}
		else return(-8);
	}

	if (retry >= 8) return(-9);

	memcpy(buf, m_buf + 4, (int)(strlen(m_buf + 4) - 1) > bufsize ? bufsize : strlen(m_buf + 4) - 1);

	return 0;
}

int CMelsecP4C::P4WriteBB(char * station, char * pcstation, char * addr, int counts, char * dat)
{
	int ret, len;
	int retry = 0;
	char data[20];

	if (m_fOpen == FALSE) return(-1);

	if ((int)strlen(dat) != counts) return(-2);

	MakeCmd(m_buf, sizeof(m_buf), station, pcstation, "BW", addr, counts, dat);

	while (retry < 8) {
		TRS232C::ClearRcvBuffer();
		ret = TRS232C::Write((BYTE*)m_buf, strlen(m_buf));

		ret = Receive(data, sizeof(data), 1);
		if (ret) return(-3);

		if (data[0] == ACK)  len = 6;
		else if (data[0] == NAK)  len = 8;
		else return(-4);

		ret = Receive(data, sizeof(data), len);
		if (ret) return(-5);

		if (len == 8) retry++;
		else break;
	}
	if (retry >= 8) return(-9);
	return 0;
}

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<READ/WRITE Multiple Point Bit<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>READ/WRITE 2C Protocol>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int CMelsecP4C::P2C4WriteW(char * station, char * pcstation, char * addr, short counts, char * msg)
{
	int ret, len;
	int retry = 0;
	char data[20];
	char initcom[5];

	initcom[0] = 4;
	initcom[1] = 0xD;
	initcom[2] = 0xA;
	initcom[3] = 0;


	if (m_fOpen == FALSE) return(-1);
	if (counts < 0 || counts > 960) return(-2);

	if (strlen(addr) + strlen(msg) + P4_HEADLEN1 >= sizeof(m_buf)) return(-10);

	if (counts * 4 != (int)strlen(msg)) return(-11);

	MakeCmd2C(m_buf, sizeof(m_buf), station, pcstation, "4", addr, counts, msg);

	while (retry <= 8) {
		if (retry) {
			TRS232C::ClearRcvBuffer();
			ret = TRS232C::Write((BYTE*)initcom, 3);
		}
		TRS232C::ClearRcvBuffer();
		ret = TRS232C::Write((BYTE*)m_buf, strlen(m_buf));

		ret = Receive(data, sizeof(data), 1);
		if (ret) {
			if (retry++ < 8)continue;
			return(-3);
		}
		if (data[0] == ACK)  len = 6;
		else if (data[0] == NAK)  len = 12;
		else {
			if (retry++ < 8)continue;
			return(-4);
		}
		ret = Receive(data, sizeof(data), len);
		if (ret) {
			if (retry++ < 8)continue;
			return(-5);
		}
		if (len == 12) retry++;

		else break;
	}
	if (retry >= 8) return(-9);
	return(0);
}
int CMelsecP4C::P2C4ReadW(char * buf, int bufsize, char * station, char * pcstation, char * addr, short counts)
{
	int ret, len;
	int retry = 0;
	char data[4096];
	char lsum[4096];
	char initcom[5];
	char st;
	char sumt[16];

	initcom[0] = 4;
	initcom[1] = 0xD;
	initcom[2] = 0xA;
	initcom[3] = 0;

	if (m_fOpen == FALSE) return(-1);
	if (counts < 0 || counts > 960) return(-2);

	MakeCmd2C(m_buf, sizeof(m_buf), station, pcstation, "2", addr, counts, "");

	while (retry <= 8) {
		if (retry) {
			TRS232C::ClearRcvBuffer();
			ret = TRS232C::Write((BYTE*)initcom, 3);
		}
		TRS232C::ClearRcvBuffer();
		ret = TRS232C::Write((BYTE*)m_buf, strlen(m_buf));

		ret = Receive(data, sizeof(data), 1);
		if (ret) {
			if (retry++ < 8)continue;
			return(-3);
		}
		st = data[0];
		if (data[0] == STX)len = counts * 4 + 9;
		else if (data[0] == NAK)  len = 12;
		else {
			if (retry++ < 8)continue;
			return(-4);
		}

		ret = Receive(data, sizeof(data), len);
		if (ret) {
			if (retry++ < 8)continue;
			return(-5);
		}
		if (st == STX) {
			memset(lsum, 0, sizeof(lsum));
			memcpy(lsum, data, len - 2);
			wsprintf(sumt, "%02X", GetSumCheckCode(lsum));
		}

		if (st == NAK || memcmp(sumt, data + len - 2, 2)) retry++;
		else break;
	}

	memcpy(buf, lsum + 6, (int)(strlen(lsum + 6) - 1) > bufsize ? bufsize : strlen(lsum + 6) - 1);

	if (retry >= 8) return(-9);

	return 0;
}

void CMelsecP4C::MakeCmd2C(char * buf, int bufsize, char * station, char * pc, char * cmd, char * addr, short counts, char * msg)
{
	char txt[10];
	int sum;

	memset(buf, 0, bufsize);
	buf[0] = ENQ;
	buf[1] = 'F';
	buf[2] = 'B';

	strcat(buf, station);
	strcat(buf, pc);

	strcat(buf, cmd);

	strcat(buf, addr);

	wsprintf(txt, "%04X", counts);
	memcpy(&buf[strlen(buf)], txt, 4);

	strcat(buf, msg);

	sum = GetSumCheckCode(&buf[1]);
	wsprintf(txt, "%02X", sum);
	memcpy(&buf[strlen(buf)], txt, 2);

	buf[strlen(buf)] = CR;
	buf[strlen(buf)] = LF;
}

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<READ/WRITE 2C Protocol<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
