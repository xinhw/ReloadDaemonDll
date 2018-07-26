
COBUCard::COBUCard()
{
	m_pCmd=NULL;
	m_pReader=NULL;
}

COBUCard::COBUCard(CCardReader *preader,ClsCommand *pcmd)
{
	m_pCmd=pcmd;
	m_pReader=preader;
}

COBUCard::~COBUCard()
{
}
//	��ʼ��
int COBUCard::init(BYTE *elf15)
{


	return 0;
}

//	���˻�
int COBUCard::personalize(BYTE *elf15,BYTE *elf16)
{
	return 0;
}

//	��Ƭ���
int COBUCard::clear(BYTE *elf15)
{
	return 0;
}


int COBUCard::read_obu(BYTE *elf01_mk,BYTE *elf01_adf01)
{
	int ret;
	char strresp[256];

	if(!validation()) return -1;

	//	0. ѡ��3F00Ŀ¼
	memset(strresp,0x00,256);
	ret = preader->RunCmd("00A40000023F00",strresp);
	if(ret) return ret;

	//	1. ��ȡEF01�ļ���	ϵͳ��Ϣ�ļ�
	memset(strresp,0x00,256);
	ret = preader->RunCmd("00B081002B",strresp);
	if(ret) return ret;
	CMisc::StringToByte(strresp,elf01_mk);

	//	2. ѡ��DF01 ADF
	memset(strresp,0x00,256);
	ret = preader->RunCmd("00A4000002DF01",strresp);
	if(ret) return ret;

	//	3. ��ȡ0015�ļ���	�����л��������ļ�
	memset(strresp,0x00,256);
	ret = preader->RunCmd("00B400000A00000000000000004F00",strresp);
	if(ret) return ret;
	CMisc::StringToByte(strresp,elf01_adf01);

	return 0;
}