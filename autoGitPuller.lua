local function commander(argToExcute)
    local result = io.popen(argToExcute)
    local strInfo = result:read("*all")
    return strInfo
end

function tryTillSucceed(arg,tryTimes)
    tryTimes = tryTimes or 1000000
    for i = 1,tryTimes,1 do
        print('argument is: ',arg)
        local res = commander(arg)
        if res ~= '' then
            print('Conduction succeeded!')
            break
        end
    end
    return
end

local pullCmd =  'git pull'
tryTillSucceed(pullCmd)
