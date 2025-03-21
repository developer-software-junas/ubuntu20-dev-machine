//*******************************************************************
//        FILE:     cyrestsales_receipt.cpp
// DESCRIPTION:     Sales receipt generation
//*******************************************************************
#include "cyreceiptrcc.h"
using std::string;
//*******************************************************************
//    FUNCTION:     assembleReceiptFooter
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool                CYReceiptRcc::assembleReceiptFooter (Json::Value& jheader,
                                                         salesReceiptType receiptType)
{
    _txtfoot = "";
    _txtbeauty = "";
    _txtloyalty = "";
    _txtfootbeauty = "";
    _txtfootloyalty = "";

    int w = g_widthReceipt;
    string tmp = "",label = "";
    //////////////////////////////////////
    //  Header settings
    if (!jheader.isMember("hdr_settings"))
        return seterrormsg("Unable to retrieve the POS settings information");
    Json::Value jhdrsettings = jheader["hdr_settings"];
    //////////////////////////////////////
    //  Cashier
    if (!jheader.isMember("settings_cashier"))
        return seterrormsg("Unable to retrieve cashier information");
    Json::Value jcashier = jheader["settings_cashier"];
    //////////////////////////////////////
    //  Register
    if (!jheader.isMember("settings_register"))
        return seterrormsg("Unable to retrieve register information");
    Json::Value jregister = jheader["settings_register"];
    //////////////////////////////////////
    //  Account list
    int numAccount = 0;
    Json::Value jacctline, jaccount;
    if (jheader.isMember("hdr_account"))  {
        jaccount = jheader["hdr_account"];
        numAccount = jaccount.size();
    } else
        jaccount = Json::nullValue;
    //////////////////////////////////////
    //  Database connection
    CYDbSql* db = _ini->dbconn ();
    if (!db->ok())
        return seterrormsg(_ini->errormsg());
    //////////////////////////////////////
    //  Gift item footer
    db->sql_reset ();
    string sql = "select * from pos_giftitem_promo_rcpt where gift_code = 888 ";
    if (!db->sql_result (sql, true))  {
        _log->logmsg("footer error", sql.c_str ());
        return seterrormsg(db->errordb ().c_str ());
    }
    if (!db->eof ())  {
        string value, align, permit_no="";
        if (jregister.isMember("permit"))
            permit_no = jregister["permit"].asString();

        do {
            value = db->sql_field_value("rcpt_line");
            align = db->sql_field_value("is_center");
            if (value.length () < 1) value = "";
            size_t npos = value.find("<PERMIT-NO>");
            if (npos != std::string::npos)
                value.replace(npos,11,permit_no.c_str());

            if (strcmp(align.c_str(),"1") == 0)
                _txtfoot += _util->cyCenterText(w,value.c_str());
            else{
                _txtfoot += value.c_str();
                _txtfoot += "\n";
            }
            db->sql_next();
        } while (!db->eof());
    }
    //////////////////////////////////////
    //  Refund message
    if (receiptType == SRT_REFUND)  {
        _txtfoot += "Customer Name :________________________\n";
        _txtfoot += "Address :______________________________\n";
        _txtfoot += "Phone No. :____________________________\n\n";
        _txtfoot += "Approving Manager :____________________\n\n\n\n";

        _txtfoot += "     Return and Exchange Slip (RES)\n";
        _txtfoot += "----------------------------------------\n";
        _txtfoot += "Please present RES upon exchange of item\n";
        _txtfoot += "         For one-time use only\n";
        _txtfoot += "       Not replaceable when lost\n";
        _txtfoot += "Can be used in Rustan`s Department Store\n";
        _txtfoot += "                  only\n";
    }
    //////////////////////////////////////
    //  Footer message(s)
    if (receiptType != SRT_POSTVOID && receiptType != SRT_REFUND) {
        char sztmp [32];
        strcpy (sztmp, "szmsg01");
        if (jhdrsettings.isMember(sztmp))  {
            _txtfoot +=  _util->cyCenterText(w, jhdrsettings[sztmp].asString());
        }
        strcpy (sztmp, "szmsg02");
        if (jhdrsettings.isMember(sztmp))  {
            _txtfoot +=  _util->cyCenterText(w, jhdrsettings[sztmp].asString());
        }
        strcpy (sztmp, "szmsg03");
        if (jhdrsettings.isMember(sztmp))  {
            _txtfoot +=  _util->cyCenterText(w, jhdrsettings[sztmp].asString());
        }
        strcpy (sztmp, "szmsg04");
        if (jhdrsettings.isMember(sztmp))  {
            _txtfoot +=  _util->cyCenterText(w, jhdrsettings[sztmp].asString());
        }
        strcpy (sztmp, "szmsg05");
        if (jhdrsettings.isMember(sztmp))  {
            _txtfoot +=  _util->cyCenterText(w, jhdrsettings[sztmp].asString());
        }
        _txtfoot += _util->cyCenterText(40,"--+++--");
        _txtfoot += RCPT_CUT;
    }
    //////////////////////////////////////
    //  Loyalty
    if (receiptType != SRT_SUSPEND && receiptType != SRT_POSTVOID &&
        receiptType != SRT_CANCEL  && receiptType != SRT_REFUND)  {
        if (_isBaMember)  {
            for (int i = 0; i < numAccount; i++)  {
                jacctline = jaccount[i];
                if (jacctline.isMember("is_beauty_addict"))  {
                    if (jacctline["is_beauty_addict"].asString() == "1")  {
                        //_txtfootbeauty += "\n\n\n";
                        label = "========= BEAUTY CARD DETAILS =========";
                        _txtfootbeauty += _util->cyCenterText(40,label.c_str());

                        label  = "Account Number: ";
                        label += jacctline["account_number"].asString();
                        _txtfootbeauty += label.c_str();
                        _txtfootbeauty += "\n";

                        label  = "Member Name: ";
                        label += jacctline["first_name"].asString();
                        label += " ";
                        label += jacctline["middle_name"].asString();
                        label += " ";
                        label += jacctline["last_name"].asString();
                        _txtfootbeauty += label.c_str();
                        _txtfootbeauty += "\n";

                        if (jacctline.isMember("ext_receipt"))  {
                            string ext_receipt = jacctline["ext_receipt"].asString();
                            if (ext_receipt.length() > 0)  {
                                _txtbeauty = ext_receipt;
                                //CONSTRUCT DATE IN RCC FORMAT
                                string trans_date,dd,mm,yyyy;
                                trans_date = jheader["transaction_date"].asString();
                                _util->date_split(trans_date.c_str(),yyyy,mm,dd);
                                label  = mm;
                                label += "/";
                                label += dd;
                                label += "/";
                                label += yyyy;

                                _txtfootbeauty += "\n";
                                tmp = "TRANSDATE: ";
                                tmp += label;
                                _txtfootbeauty += _util->cyCenterText(w,tmp);

                                tmp = jheader["transaction_number"].asString();
                                if (tmp.length() < 1) tmp = "0";
                                int trx = atoi(tmp.c_str ());
                                char sztmp [32];
                                sprintf (sztmp, "%015d", trx);


                                tmp = "POS # ";
                                tmp += jregister["register_num"].asString();
                                label = "TRANS. NO. ";
                                label += sztmp;
                                _txtfootbeauty += _util->cyLRText(10,tmp,30,label);

                                _txtfootbeauty += "\n\n\n\n\n";
                                _txtfootbeauty += ext_receipt;
                            }
                        }
                        _txtfootbeauty += _util->cyAcrossText(40,"=");
                        _txtfootbeauty += RCPT_CUT;

                        break;
                    }
                }
            }
        }
        if (_isFspMember)  {
            for (int i = 0; i < numAccount; i++)  {
                jacctline = jaccount[i];
                if (jacctline.isMember("is_loyalty"))  {
                    if (jacctline["is_loyalty"].asString() == "1")  {
                        if (jacctline.isMember("ext_receipt"))  {
                            string ext_receipt = jacctline["ext_receipt"].asString();
                            if (ext_receipt.length() > 0)  {
                                _txtloyalty = ext_receipt;
                            }
                        }
                        if (_txtloyalty.length() > 0)  {
                            string totalPoints = jacctline["points_balance"].asString();
                            string totalEPoints = jacctline["epurse_balance"].asString();

                            //_txtfootloyalty += "\n\n\n";
                            label = "============= FSP DETAILS =============";
                            _txtfootloyalty += _util->cyCenterText(40,label.c_str());

                            label  = "Account Number: ";
                            label += jacctline["account_number"].asString();
                            _txtfootloyalty += label.c_str();
                            _txtfootloyalty += "\n";

                            label  = "Member Name: ";
                            label += jacctline["first_name"].asString();
                            label += " ";
                            label += jacctline["middle_name"].asString();
                            label += " ";
                            label += jacctline["last_name"].asString();
                            _txtfootloyalty += label.c_str();
                            _txtfootloyalty += "\n\n";

                            label  = "TOTAL PURCHASE: ";
                            tmp = FMTNumberComma(_totalNetSales);
                            _txtfootloyalty += _util->cyLRText(20, label.c_str(),20,tmp.c_str());

                            //CONSTRUCT DATE IN RCC FORMAT
                            string trans_time,trans_date,dd,mm,yyyy;
                            trans_date = jheader["transaction_date"].asString();
                            trans_time = jheader["transaction_time"].asString();
                            _util->date_split(trans_date.c_str(),yyyy,mm,dd);
                            label  = mm;
                            label += "/";
                            label += dd;
                            label += "/";
                            label += yyyy;
                            label += "-";
                            label += _util->time_colon(trans_time.c_str());

                            tmp = "TERM NO. ";
                            tmp += jheader["register_number"].asString();
                            _txtfootloyalty += _util->cyLRText(20, label.c_str(),20,tmp.c_str());

                            string trx_num = jheader["transaction_number"].asString();
                            label  = "TRANS NO. ";
                            label += trx_num;

                            jcashier = jheader["settings_cashier"];

                            tmp  = jcashier["last_name"].asString();
                            tmp +=  ", ";
                            tmp +=  jcashier["first_name"].asString();
                            _txtfootloyalty += _util->cyLRText(20, label.c_str(),20,tmp.c_str());
                            _txtfootloyalty += "\n\n\n\n\n";
                            _txtfootloyalty += _txtloyalty;
                            _txtfootloyalty += _util->cyAcrossText(40,"=");
                            _txtfootloyalty += RCPT_CUT;
                        }

                        break;
                    }
                }
            }
        }
    }
    _txtfoot += _util->cyCenterText(w, " ");
    _receipt += _txtfoot;

    return true;
}
