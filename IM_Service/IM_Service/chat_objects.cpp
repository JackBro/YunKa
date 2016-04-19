#include "stdafx.h"
#include "chat_objects.h"


CChatObjects::CChatObjects()
{
}


CChatObjects::~CChatObjects()
{
}

void CChatObjects::OnReceive(void* pHead, void* pData)
{
	TCP_PACK_HEADER TcpPackHead;

	int nPackHeadLen = COM_HEAD_PACK::GetHeadLen();
	int nTcpPackHeadLen = sizeof(TCP_PACK_HEADER);

	int nError = 0;
	int nErrType = 0;
	CPackInfo packinfo;
	bool bLast = false;

	TcpPackHead = *((TCP_PACK_HEADER *)pHead);
	assert(TcpPackHead.len <= PACKMAXLEN);


	char *RecvBuf = (char*)pData;

	PACK_HEADER head = *((PACK_HEADER*)RecvBuf);

	g_WriteLog.WriteLog(C_LOG_TRACE, "Mainfrm recv Cmd:%.4x", head.cmd);

	switch (head.cmd)
	{
	case CMD_COM_ACK:
		break;
	case CMD_SRV_REP_USERINFO:  //�յ��û���Ϣ��
		//nError = RecvSrvRepUserinfo(head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_SRV_CONF_LOGOFF:   //�ǳ�ȷ�ϰ�
		//nError = RecvSrvConfLogOff(head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_SRV_STATUS_FRDONLINE: //��������
		//nError = RecvSrvStatusFrdOnline(head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_SRV_STATUS_FRDOFFLINE: //��������
		//nError = RecvSrvStatusFrdOffline(head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_SRV_STATUS_USER_FORM:   //���Ѹ�������״̬
		//nError = RecvSrvStatusUserForm(head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_FLOAT_SHARELIST:  //�յ����Ѷ����б�
		//nError = RecvFloatShareList(head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_COM_SEND_MSG:
		//nError = RecvComSendMsg(head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_FLOAT_CREATECHAT: //����Ự
		//nError = RecvFloatCreateChat(head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_FLOAT_CHATMSG:  //�Ự��Ϣ
		//nError = RecvFloatChatMsg(head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_FLOAT_CHATMSG_ACK: //������ϢӦ��
		//nError = RecvFloatChatMsgAck(head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_FLOAT_ACCEPTCHAT:  //���ܻỰ
		//nError = RecvFloatAcceptChat(head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_FLOAT_TRANSQUEST:		//�ȴ�Ӧ��  �Ựת������
		//nError = RecvFloatTransQuest(head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_FLOAT_TRANSFAILED:   //�ȴ�Ӧ��  �Ựת��ʧ��
		//nError = RecvFloatTransFailed(head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_FLOAT_INVITE_REQUEST://�ȴ�Ӧ��  ��������
		//nError = RecvInviteRequest(head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_FLOAT_INVITE_RESULT://�ȴ�Ӧ��  ������
		//nError = RecvInviteResult(head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_FLOAT_KEFU_RELEASE: //��ϯ�����ͷŻỰ
		//nError = RecvFloatKefuRelease(head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_FLOAT_CMDERROR:  //�ȴ�Ӧ��  ����ʧ��
		//nError = RecvFloatCMDError(head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_FLOAT_CLOSECHAT: //�Ự�ر�
		//nError = RecvFloatCloseChat(head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_FLOAT_LISTCHAT:  //�Ự�б�
		//nError = RecvFloatListChat(head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_FLOAT_CHATINFO:  //�Ự��ϸ��Ϣ
		//nError = RecvFloatChatInfo(head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_EVENT_ANNOUNCEMENT:  //�յ���Ϣͨ��
		//nError = RecvEventAnnouncement(head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_SRV_UPDATE_SUCC:  //������Ϣ�ɹ�
		//nError = RecvSrvUpdateSucc(head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_SRV_UPDATE_FAIL:  //������Ϣʧ��
		//nError = RecvSrvUpdateFail(head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_SRV_SERVER_COPY:   //�û�����ص�½
		//nError = RecvSrvDonw(head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_SRV_REP_TRANSFERCLIENT://			0x01B8  // 440 ת����ʱ�û��ɹ�ʧ��
		//nError = RecvRepTransferClient(head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;
	case CMD_SRV_TRANSFERCLIENT:	//0x01C2  ����ת�ƻỰ���û�����
		//nError = RecvTransferClient(head, RecvBuf + nPackHeadLen, TcpPackHead.len - nPackHeadLen);
		break;

	default:
		break;
	}
	return;
}
