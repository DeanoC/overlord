local function approx(a, b, precision)
    local d = a - b + 1e-10
    if d > precision or d < -precision then
        return false
    else
        return true
    end
end

do
    local test = image.create2D(16, 16, "R8G8B8A8_UNORM")
    if test == nil then
        print("create2D fail")
    end
    for y = 1, 16 do
        for x = 1, 16 do
            local i = (y - 1) * 16 + (x - 1)
            test:setPixelAt(i, (x - 1.0) / 15.0, (y - 1.0) / 15.0, (x - 1.0) / 15.0, 1.0)
        end
    end
    test:saveAsKTX("test_save_col_16x16.ktx")

    local loadtest, success = image.load("test_save_col_16x16.ktx")
    assert(success)

end

do
    local test = image.create(884, 406, 1, 1, "R8G8B8A8_UNORM")
    if test == nil then
        print("unable to be create image")
    end
    local w = test:width();
    local h = test:height();
    local d = test:depth();
    local s = test:slices();
    local format = test:format();
    local flags = test:flags()

    if w ~= 884 then
        print("create image is " .. w .. " wide it should be 884")
    end
    if h ~= 406 then
        print("create image is " .. h .. " high it should be 406")
    end
    if d ~= 1 then
        print("create image is " .. d .. " depth it should be 1")
    end
    if s ~= 1 then
        print("create image is " .. s .. " slices it should be 1")
    end
    if format ~= "R8G8B8A8_UNORM" then
        print("create image is " .. format .. " and should be R8G8B8A8_UNORM")
    end
    if flags.Cubemap == true then
        print("create image marked as a Cubemaps and shouldn't be")
    end
    if flags.HeaderOnly == true then
        print("create image marked as a header only  and shouldn't be")
    end
    -- should be a completely black with 0 alpha image
    test:saveAsPNG("test_createsave.png")
end

do
    local test = image.createNoClear(444, 333, 1, 1, "R8G8B8A8_UNORM")
    if test == nil then
        print("unable to be create image")
    end
    local w = test:width();
    local h = test:height();
    local d = test:depth();
    local s = test:slices();
    local format = test:format();
    local flags = test:flags()

    if w ~= 444 then
        print("create image is " .. w .. " wide it should be 444")
    end
    if h ~= 333 then
        print("create image is " .. h .. " high it should be 333")
    end
    if d ~= 1 then
        print("create image is " .. d .. " depth it should be 1")
    end
    if s ~= 1 then
        print("create image is " .. s .. " slices it should be 1")
    end
    if format ~= "R8G8B8A8_UNORM" then
        print("create image is " .. format .. " and should be R8G8B8A8_UNORM")
    end
    if flags.Cubemap == true then
        print("create image marked as a Cubemaps and shouldn't be")
    end
    if flags.HeaderOnly == true then
        print("create image marked as a header only  and shouldn't be")
    end
    -- save could be zeroed or random
    test:saveAsPNG("test_createsave1.png")
end

-- create a pow2 2D texture fill with set pixels save as ktx
do
    local test, okay = image.create2D(16, 16, "R8G8B8A8_UNORM")
    if okay ~= true then
        error("create2D fail")
    end
    for y = 1, 16 do
        for x = 1, 16 do
            local i = (y - 1) * 16 + (x - 1)
            test:setPixelAt(i, (x - 1.0) / 15.0, (y - 1.0) / 15.0, (x - 1.0) / 15.0, 1.0)
        end
    end

    for y = 0, 15 do
        for x = 0, 15 do
            local i = y * 16 + x
            local r, g, b, a = test:getPixelAt(i)
            if not approx(r, x / 15.0, 1e-5) or not approx(g, y / 15.0, 1e-5) or not approx(b, x / 15.0, 1e-5) or not approx(a, 1.0, 1e-5) then
                print("create2D get pixel failed <" .. x .. "," .. y .. ">")
                print(r .. "," .. g .. "," .. b .. "," .. a)
                print(x / 15.0 .. "," .. y / 15.0 .. "," .. x / 15.0 .. "," .. 1.0)
                break
            end
        end
    end
    test:saveAsKTX("setpixel_rgba8_16x16.ktx")
end

-- create a 256x256 image, set pixels save as png, reload and check every pixel was set and saved correctly
-- checks manual index calculations and calculateIndex function
do
    local test, okay = image.createNoClear(256, 256, 1, 1, "R8G8B8A8_UNORM")
    if test == nil then
        print("unable to be create image")
    end

    for y = 1, 256 do
        for x = 1, 256 do
            local i = (y - 1) * 256 + (x - 1)
            test:setPixelAt(i, (x - 1.0) / 255.0, (y - 1.0) / 255.0, (x - 1.0) / 255.0, (y - 1.0) / 255.0)
        end
    end
    test:saveAsPNG("setpixel_rgba8_256x256.png")

    local loadtest, okay = image.load("setpixel_rgba8_256x256.png")
    for y = 1, 256 do
        for x = 1, 256 do
            local i = (y - 1) * 256 + (x - 1)
            local r, g, b, a = loadtest:getPixelAt(i)
            if not approx(r, (x - 1.0) / 255.0, 1e-5) then
                print(x .. ", " .. y .. ": " .. r .. " red incorrect in setpixel_rgba8_256x256.png")
            end
            if not approx(g, (y - 1.0) / 255.0, 1e-5) then
                print(x .. ", " .. y .. ": " .. g .. " green incorrect in setpixel_rgba8_256x256.png")
            end
            if not approx(b, (x - 1.0) / 255.0, 1e-5) then
                print(x .. ", " .. y .. ": " .. b .. " blue incorrect in setpixel_rgba8_256x256.png")
            end
            if not approx(a, (y - 1.0) / 255.0, 1e-5) then
                print(x .. ", " .. y .. ": " .. a .. " alpha incorrect in setpixel_rgba8_256x256.png")
            end
        end
    end

    for y = 1, 256 do
        for x = 1, 256 do
            local i = loadtest:calculateIndex(x - 1, y - 1, 0, 0)

            local r, g, b, a = loadtest:getPixelAt(i)
            if not approx(r, (x - 1.0) / 255.0, 1e-5) then
                print(x .. ", " .. y .. ": " .. r .. " red incorrect in calculateIndex test")
            end
            if not approx(g, (y - 1.0) / 255.0, 1e-5) then
                print(x .. ", " .. y .. ": " .. g .. " green incorrect in calculateIndex test")
            end
            if not approx(b, (x - 1.0) / 255.0, 1e-5) then
                print(x .. ", " .. y .. ": " .. b .. " blue incorrect in calculateIndex test")
            end
            if not approx(a, (y - 1.0) / 255.0, 1e-5) then
                print(x .. ", " .. y .. ": " .. a .. " alpha incorrect in calculateIndex test")
            end
        end
    end
end