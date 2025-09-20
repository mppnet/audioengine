if [ -n "$PASSWORD" ]; then
    user_pw="$PASSWORD"
else
    read -sp "Enter password: " user_pw
    echo
fi

cd pkg && zip -r ../audioengine2.zip . -x .gitignore -x audioengine2_bg.wasm.d.ts -x README.md -x package.json && cd ..
curl -F f=@audioengine2.zip "https://files.sad.ovh/public?want=url&replace&pw=$user_pw";
rm audioengine2.zip
