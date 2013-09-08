    git clone https://github.com/zeMirco/sf-city-lots-json.git data
    cp data/citylots.json stress.json

    git clone https://github.com/EricMCornelius/UberTest.git UberTest

    posh generate
    posh build
    
    ./tests/json/bin/JsonTest.tsk
