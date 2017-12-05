void main(void) { 
    HCRYPTMSG hMsg;
    BYTE* pbContent;     // a byte pointer to the message
    DWORD cbContent;     // the size of message
    DWORD cbEncodedBlob;
    BYTE* pbEncodedBlob;

    pbContent = (BYTE*) "Security is our only business";
    cbContent = strlen((char *) pbContent) + 1;

    printf("The original message => %s\n",pbContent);  

    cbEncodedBlob = CryptMsgCalculateEncodedLength(
                MY_ENCODING_TYPE,           // message encoding type
                0,                          // flags
                CMSG_DATA,                  // message type
                NULL,                       // pointer to structure
                NULL,                       // inner content object ID
                cbContent);                 // size of content
    printf("The length of the data has been calculated. \n");

    pbEncodedBlob = (BYTE *) malloc(cbEncodedBlob);
    printf("Memory has been allocated for the signed message. \n");

    hMsg = CryptMsgOpenToEncode(
                MY_ENCODING_TYPE,           // encoding type
                0,                          // flags
                CMSG_DATA,                  // message type
                NULL,                       // pointer to structure
                NULL,                       // inner content object ID
                NULL);                      // stream information (not used)
    printf("The message to be encoded has been opened. \n");

    CryptMsgUpdate(
                hMsg,                       // handle to the message
                pbContent,                  // pointer to the content
                cbContent,                  // size of the content
                TRUE);                      // last call
    printf("Content has been added to the encoded message. \n");

    CryptMsgGetParam(
                hMsg,                       // handle to the message
                CMSG_BARE_CONTENT_PARAM,    // parameter type
                0,                          // index
                pbEncodedBlob,              // pointer to the BLOB
                &cbEncodedBlob);            // size of the BLOB
    printf("Message encoded successfully. \n");

    CryptMsgClose(hMsg);
    free(pbEncodedBlob);
} //  End of main
