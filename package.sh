read -sp "Enter password: " user_pw
echo

cd pkg && zip -r ../audioengine2.zip . -x .gitignore && cd ..
curl -F f=@audioengine2.zip "https://files.sad.ovh/public?want=url&replace&pw=$user_pw";
rm audioengine2.zip
