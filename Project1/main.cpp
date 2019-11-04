#include "CMelsecP4C.h"
#include "CMelP3E.h"
#include "CMelP1E.h"
#include "CMelP4Ex.h"
#include "CBarwellPLC.h"
#include <stdio.h>
#include <iostream>
#include <stdint.h>
#include <conio.h>

int main(void)
{

	int ret;
	
    CMelsecP4C cMel; //COM

	//CMelsecP4Ex cMel;

	//CBarwellPLC cBarwell; //COM

	//CMelsecP3E cMel3E;


	//CMelsecP1E cMel1E; 

	char buf[1020];

	int a = cMel.Init(1, 115200, 8, 2, 2, 0);

	ret = cMel.P4WriteB("00", "FF", "M0400", 1);


	//cMel.P4ReadB("00", "FF", "B0210");

	//cMel.P4WriteW("00", "FF", "B0210", 1, "2347");
	//if (ret)
	//	return;

	//P3E

	/*cMel3E.Init();

	cMel3E.SetMode(FALSE); */

	//ret = cMel3E.P3EWrite("10.203.83.61", 25884, DEV_WORD, "D*000001", 1, "12AB");
	//ret=m_plc.P3EWrite(addr,m_srvport,DEV_WORD, "B*000100",1,data);

	//ret = cMel3E.P3ERead("10.203.83.65", 25884, buf, sizeof(buf), DEV_WORD, "D*000000", 1);

	//ret = cMel3E.foo("500000FF03FF000018001004010000D*0000000001", buf);
	//ret = cMel3E.P3EReadAscii("10.203.83.61", 25884, buf, sizeof(buf), DEV_WORD, "D*000000", 1, 0);

	//P1E
	//ret=m_plcFx.P1EWrite( addr,m_srvport,DEV_BIT,"M ",plcm,1,smode);
	//ret = m_plcFx.P1ERead(addr,m_srvport,buf, sizeof(buf),DEV_BIT,"W*000100",plcm,1);
	//ret = m_plcFx.P1ERead(addr,m_srvport,buf, sizeof(buf),DEV_WORD,"D ",plcm,1);
	//ret = m_plcFx.P1ERead(addr, m_srvport, buf, sizeof(buf), DEV_WORD, "W ", atoi(plcm), 1);
	//ret = m_plcFx.P1ERead(addr,m_srvport,buf, sizeof(buf),DEV_WORD,"D ",501,B1);
	//ret=m_plcFx.P1EWrite( addr,m_srvport,DEV_BIT,"S ",plcm,1,smode);

	//cMel1E.Init(1);
	//ret = cMel1E.P1ERead("10.203.83.61", 25884, buf, sizeof(buf), DEV_WORD, "D ", 0, 1);
	//ret = cMel1E.P1EWrite("10.203.83.61", 25884, DEV_WORD, "D ", 0, 1, "0008");

	//ret = cMel1E.P1ERead("10.203.83.61", 25884, buf, sizeof(buf), DEV_WORD, "D ", 0, 1);

	if (ret) 
		return(-3);

	
	_getch();
	
}
