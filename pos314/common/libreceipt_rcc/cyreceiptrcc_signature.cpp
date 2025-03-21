//*******************************************************************
//        FILE:     cyrestsales_receipt.cpp
// DESCRIPTION:     Sales receipt generation
//*******************************************************************
#include "cyreceiptrcc.h"
using std::string;
//*******************************************************************
//    FUNCTION:     assembleReceiptSignature
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool                CYReceiptRcc::assembleReceiptSignature (Json::Value& jheader,
                                            salesReceiptType receiptType)
{

    if (receiptType == salesReceiptType::SRT_SUSPEND )
        return 0;

    char szTmp[255];
    _txtsig = "";
    _isBaMember = false;
    _isFspMember = false;
    Json::Value jacct,jtemp,jtemp2;
    string tmp3="",tmp2="",newLine = "\n";
    //////////////////////////////////////
    //CHECK IF FSP OR BA
    string info, label,tmp,txtacct2 = "";
    bool isFsp = false;
    bool isBeauty = false;
    bool isLine = false;
    Json::Value jaccount,jcashier;
    if (jheader["hdr_account"] == Json::nullValue)
        return true;
    jaccount = jheader["hdr_account"];

    int j = jaccount.size();

    //RETRIVE FSP DETAILS
    for (int i = 0; i < j; i++)  {
        jacct = jaccount[i];
        if (jacct["is_loyalty"].asString() == "1") {
            isFsp = true;

        }

        if (isFsp){
            string totalPoints = jacct["points_balance"].asString();
            _log->logmsg("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^",totalPoints.c_str ());
            string totalEPoints = jacct["epurse_balance"].asString();


            double dpurse = _util->stodsafe(totalEPoints);

            label = "============= FSP DETAILS =============";
            _txtsig += _util->cyCenterText(40,label.c_str());

            label  = "Account Number: ";
            label += jacct["account_number"].asString();
            _txtsig += label.c_str();
            _txtsig += "\n";

            label  = "Member Name: ";
            label += jacct["first_name"].asString();
            label += " ";
            label += jacct["middle_name"].asString();
            label += " ";
            label += jacct["last_name"].asString();
            _txtsig += label.c_str();
            _txtsig += "\n";

            long lpoints = atol(totalPoints.c_str ());
            sprintf(szTmp,"%ld",lpoints);
            totalPoints =  _util->fmt_number_comma(szTmp);

            label  = "Current Available Points: ";
            if (totalPoints.length() > 4)
                totalPoints[totalPoints.length()-3]='\0';
            label += totalPoints;
            _txtsig += label.c_str();
            _txtsig += "\n";

            sprintf(szTmp,"%.02f",dpurse);
            totalEPoints = _util->fmt_number_comma(szTmp);
            if (totalEPoints.length() > 4)
                totalEPoints[totalEPoints.length()-3]='\0';
            label  = "Current Available E-Purse: ";
            label += totalEPoints;


            _txtsig += label.c_str();
            _txtsig += "\n";

            isFsp = false;
            isLine = true;
            _isFspMember = true;
        }
    }
    //RETRIVE BA
    for (int i = 0; i < j; i++)  {
        jacct = jaccount[i];

        if (jacct["is_beauty_addict"].asString() == "1") {
            isBeauty = true;
        }

        if (isBeauty){
            string totalPoints = jacct["points_balance"].asString();
            string pointsEarned = jacct["points_earned"].asString();

            double dpoints = _util->stodsafe(totalPoints);
            double dearned = _util->stodsafe(pointsEarned);

            _txtsig += "\n";
            label = "========= BEAUTY CARD DETAILS =========";
            _txtsig += _util->cyCenterText(40,label.c_str());

            label  = "Account Number: ";
            label += jacct["account_number"].asString();
            _txtsig += label.c_str();
            _txtsig += "\n";

            label  = "Member Name: ";
            label += jacct["first_name"].asString();
            label += " ";
            label += jacct["middle_name"].asString();
            label += " ";
            label += jacct["last_name"].asString();
            _txtsig += label.c_str();
            _txtsig += "\n";

            sprintf(szTmp,"%ld",(long)dearned);
            pointsEarned = _util->fmt_number_comma(szTmp);            
            label  = "Current Earned Points: ";
            label += pointsEarned;
            _txtsig += label.c_str();
            _txtsig += "\n";

            sprintf(szTmp,"%ld",(long)dpoints);
            totalPoints = _util->fmt_number_comma(szTmp);
            label  = "Total Earned Points: ";
            label += totalPoints;
            _txtsig += label.c_str();
            _txtsig += "\n";

            isBeauty = false;
            isLine = true;
            _isBaMember = true;

        }

    }

    if (isLine)
        _txtsig += _util->cyAcrossText(40,"=");


    //END OF FSP AND BA
    //////////////////////////////////////
    //  Beauty addict???
    if (jheader.isMember("hdr_trxaccount"))  {
        Json::Value jtrxacct = jheader["hdr_trxaccount"];
        if (jtrxacct["account_type_code"].asString() == "BA")  {

            //_txtsig += "\n\n\n";
            label = "========= BEAUTY CARD DETAILS ==========";
            _txtsig += _util->cyCenterText(40,label);

            label  = "Account Number: ";
            label += jtrxacct["account_number"].asString();
            _txtsig += label.c_str();
            _txtsig += "\n";

            label  = "Member Name   : ";
            label += jtrxacct["first_name"].asString();
            label += " ";
            label += jtrxacct["middle_name"].asString();
            label += " ";
            label += jtrxacct["last_name"].asString();
            _txtsig += label.c_str();
            _txtsig += "\n";

            label  = "Points balance: ";
            tmp = jtrxacct["points_balance"].asString();
            double dpts = _util->stodsafe(tmp.c_str ());
            label += jtrxacct["points_balance"].asString();
            if (dpts != 0.00)  {
                _txtsig += label.c_str();
                _txtsig += "\n";
            }
            _txtsig += _util->cyAcrossText(40,"=");
        }
    }

    if (receiptType != salesReceiptType::SRT_POSTVOID) {
        //IF CUSTOMER IS TAG
        string fspTagMobile = jheader["crm_mobile"].asString();
        if(strlen(fspTagMobile.c_str()) > 0){
            _txtsig += newLine.c_str();
            _txtsig += "I have given full consent to receive\n";
            _txtsig += "communication for the purpose of being\n";
            _txtsig += "invited to join the Rustan's loyalty\n";
            _txtsig += "program and receiving information about\n";
            _txtsig += "marketing programs from Rustan's.\n";
            _txtsig += newLine.c_str();
        }

        jacct = jheader["hdr_tax_receipt"];
        jtemp = jheader["hdr_trxaccount"];

        jtemp2 = jheader["hdr_trxaccount"];
        tmp3 = jacct["company_name"].asString();
        //tmp3 += " ";
        //tmp3 += jacct["last_name"].asString();

        //if (jacct["company_name"].asString().length() < 1 && jacct["last_name"].asString().length() < 1){
        if (jacct["company_name"].asString().length() < 1){
            _txtsig += "Customer Name:   _______________________\n";
            _txtsig += "Address:         _______________________\n";
            _txtsig += "TIN:             _______________________\n";
            _txtsig += "Business Style:  _______________________\n";
            _txtsig += newLine.c_str();
        } else {
            _txtsig += newLine.c_str();

            if (tmp3.length()+17 <= 40){
                tmp2 = "Customer name:   ";
                tmp2 += tmp3.c_str();
                tmp2 += newLine.c_str();

                _txtsig += tmp2.c_str();
            } else {
                tmp2 = "Customer name:   ";
                tmp2 += jtemp2["first_name"].asString();
                tmp2 += newLine.c_str();
                tmp2 += jtemp2["last_name"].asString();
                tmp2 += newLine.c_str();

                _txtsig += tmp2.c_str();
            }

            jtemp2 = jheader["hdr_tax_receipt"];
            tmp3 = jtemp2["address1"].asString();
            tmp3 += " ";
            tmp3 += jtemp2["address2"].asString();

            if (jtemp2["address1"].asString().length() < 1 && jtemp2["address2"].asString().length() < 1){
                _txtsig += "Address: _______________________________";
                _txtsig += newLine.c_str();
            } else if (tmp3.length() + 9 <=40){
                _txtsig += "Address: ";
                _txtsig += tmp3.c_str();
                _txtsig += newLine.c_str();
            } else {
                _txtsig += "Address: ";
                _txtsig += jtemp2["address1"].asString();
                _txtsig += newLine.c_str();
                _txtsig += jtemp2["address2"].asString();
                _txtsig += newLine.c_str();
            }

            tmp3 = jtemp2["tin"].asString();
            if (tmp3.length() < 1) {
                _txtsig += "TIN : __________________________________";
                _txtsig += newLine.c_str();
            } else {
                _txtsig += "TIN: ";
                _txtsig += tmp3.c_str();
                _txtsig += newLine.c_str();
            }

            tmp3 = jtemp2["customer_name"].asString();

            if (tmp3.length() < 1) {
                _txtsig += "Business style:_________________________";
                _txtsig += newLine.c_str();
                _txtsig += newLine.c_str();
            } else {
                _txtsig += "Business style: ";
                _txtsig += tmp3.c_str();
                _txtsig += newLine.c_str();
                _txtsig += newLine.c_str();
            }
        }


        jtemp2 = jheader["header_transtype_discount"];

        if (jtemp["is_senior"].asString() == "1" || jtemp2["discount_code"].asString() == "SENIOR") {

            jtemp2 = jheader["hdr_trxaccount"];
            tmp3 = jtemp2["first_name"].asString();
            tmp3 += " ";
            tmp3 += jtemp2["last_name"].asString();

            _txtsig += _util->cyCenterText(40,"DISCOUNT CLAIM");
            _txtsig += newLine.c_str();
            _txtsig += "Signature: _____________________________\n";
            _txtsig += "Discount: ";

            jtemp2 = jheader["header_transtype_discount"];
            _txtsig += jtemp2["description"].asString();
            _txtsig += newLine.c_str();

            if (tmp3.length()+6 <=40){
                _txtsig += "Name: ";
                _txtsig += tmp3.c_str();
                _txtsig += newLine.c_str();
            } else {
                _txtsig += "Name: ";
                _txtsig += jtemp2["first_name"].asString();
                _txtsig += newLine.c_str();
                _txtsig += jtemp2["last_name"].asString();
                _txtsig += newLine.c_str();
            }
            _txtsig += "OSCA/PWD/PNSTM/SPIC ID No: _____________\n";
        }
    }

    _receipt += _txtsig;
    return true;
}
