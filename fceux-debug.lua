local OBJECT_MAX = 34
local OBJECT_BYTES = 6 -- number of bytes used by objects
local TRIGGER_MAX = 14
local TRIGGER_BYTES = 5
local PLAYER_HEIGHT = 16

local BUILD_DIR = './build/'
local ASSETS_DIR = './assets/'
local LABEL_FILE = 'game.lbl'

local LEVEL_MASK = 0x3F

local mem = {}
local total_triggers

local function text(x, y, str, bg_color, text_color)
    gui.text(x, y, str, text_color or "black", bg_color or "#ffffffaa")
end

-- find a memory location base on name or byte value
local function memloc(location)
    local mem_loc = location
    if type(location) == "string" then
        -- check if we need underscore for C variable
        if mem[location] == nil then
            location = "_" ..location
        end
        mem_loc = mem[location]
    end

    if not mem_loc then
        print("Invalid location: " .. location)
        mem_loc = 0
    end
    return mem_loc
end

local function membyte(location, offset)
    return memory.readbyte(memloc(location) + (offset or 0))
end

local function memword(location, offset)
    return memory.readword(memloc(location) + (offset or 0))
end

local function csv_next(f)
    local result = {}
    local row
    while true do
        row = f:read("*line")
        if row == nil then
            return nil
        end
        -- skip comments
        if string.sub(row, 1, 1) ~= '#' then
            break
        end
    end
    for word in string.gmatch(row, '([^,]+)') do
        table.insert(result, word)
    end
    return result
end

local function csv_row(f, headers)
    local row = csv_next(f)
    if row == nil then
        return nil
    end
    local data = {}
    for i,k in ipairs(headers) do
        data[k] = row[i]
    end
    return data
end

local function read_mem_locations()
    local f = io.open(BUILD_DIR .. LABEL_FILE, "r")
    if f == nil then error("No label file found!") end
    while true do
        local line = f:read("*line")
        if line == nil then
            break
        end
        local value, name = string.match(line, "al ([0-9A-F]+) .([_a-zA-Z0-9]*)")
        if name ~= nil then
            mem[name] = tonumber(value, 16)
        end
    end
end

local function object_sort(a, b)
    if a.genre == b.genre then
        return a.name < b.name
    else
        if a.genre == "_global" then
            return true
        elseif b.genre == "_global" then
            return false
        else
            return a.genre < b.genre
        end
    end
end

local function read_csv(filename)
    local f = io.open(filename, "r")
    if f == nil then error("CSV not found!") end
    local headers = csv_next(f)
    csv_next(f) -- info
    local data = {}
    while true do
        local row = csv_row(f, headers)
        if row == nil then
            break
        end
        table.insert(data, row)
    end
    return data
end

local function read_levels()
    local f = io.open(ASSETS_DIR .. "levels/levels.lvl", "r")
    if f == nil then error("Level file not found!") end
    local levels = {}
    while true do
        local line = f:read("*line")
        if line == nil then
            break
        end
        -- remove comments
        line = line:gsub("%s?#[^\n\r]+", "")
        if line ~= "" then
            table.insert(levels, line)
        end
    end
    return levels
end

local HEX = "0123456789ABCDEF"
local COLORS = {
    "#BA94D1",
    "#CE7777",
    "#CEEDC7",
    "#86C8BC",
    "#A8D1D1",
    "#F675A8",
    "#AEBDCA",
    "#8FBDD3",
}

local levels = read_levels()
local triggers = read_csv(ASSETS_DIR .. "data/trigger.csv")
local objects = read_csv(ASSETS_DIR .. "data/object_types.csv")
table.sort(objects, object_sort)

local function pad_to(str, len)
    local remaining = len - str:len()
    if remaining < 0 then
        remaining = 0
    end
    return ("0"):rep(remaining) .. str
end

local function to_base(num, base)
    local result = ""
    while num ~= 0 do
        local i = (num % base) + 1
        result = HEX:sub(i, i) .. result
        num = math.floor(num / base)
    end
    return result
end

local function to_hex(num, pad)
    pad = pad or 2
    return pad_to(to_base(num, 16), pad)
end

local function to_binary(num)
    return pad_to(to_base(num, 2), 8)
end

local function bit_on(byte, bit)
    local bitmask = 2 ^ bit
    return bitmask == AND(byte, bitmask)
end

local function display_bit_boxes(x, y, byte, color)
    for bit = 1,8 do
        local bit0 = bit - 1
        local _x = x + bit0 * 3
        -- from most significant to least
        if bit_on(byte, 7-bit0) then
            gui.box(_x, y, _x + 1, y + 1, color)
        end
    end
end

local function display_ppu(x, y)
    text(x, y, "CTRL: " .. to_binary(membyte(0x2000)))
    text(x, y+9, "MASK: " .. to_binary(membyte(0x2001)))
    text(x, y+18, "SCRL: " .. to_binary(membyte(0x2005)))
    text(x, y+27, "ADDR: " .. to_binary(membyte(0x2006)))
end

local function randomize_memory(prgram)
    for addr = 0,0x0800 do
        memory.writebyte(addr, math.random(0, 255))
    end
    if prgram then
        for addr = 0x4000,0xFFFF do
            memory.writebyte(addr, math.random(0, 255))
        end
    end
end

local trigger_defines = {
    level_zone = {
        y = 16,
        h = 208
    },
    camera_lock = {
        y = 24,
        h = 192
    },
}

local function level_alt(level)
    local alt = ""
    if AND(level, 0x80) == 0x80 then
        alt = "+"
    end
    if AND(level, 0x40) == 0x40 then
        alt = "%"
    end
    return alt
end

local function level_id(value)
    return level_alt(value) .. tostring(AND(value, LEVEL_MASK))
end

local function level_name(level)
    level = AND(level, LEVEL_MASK) -- level mask
    local name = levels[level + 1]
    if name == nil then
        name = level_id(level)
    end
    return name
end

local function level_short_name(level)
    local lvl_id = AND(level, LEVEL_MASK)
    local lvl_name = levels[lvl_id + 1]
    if lvl_name then
        local x = lvl_name:find("/")
        if x then
            lvl_name = lvl_name:sub(x + 1)
        end
    else
        lvl_name = tostring(lvl_id)
    end
    return level_alt(level) .. lvl_name
end

local function display_trigger(camera_x, trigger, trigger_type, trigger_x, trigger_state)
    local screen_x = trigger_x - camera_x

    if screen_x >= 0 or screen_x <= 256 then
        local status = to_hex(trigger_state)
        local color = COLORS[(trigger_type % #COLORS) + 1]
        local name = trigger.name

        local def = trigger_defines[name]
        local obj_y = def.y
        local w, h = 32, def.h
        if name == "scroll_stop" then
            local min = math.floor(trigger_state / 16)
            local max = trigger_state % 16
            status = min .. "-" .. max
        elseif name == "level_zone" then
            local level = trigger_state
            local alt = level_alt(level)
            local lvl_name = level_name(level)
            if lvl_name then
                local x = lvl_name:find("/")
                name = lvl_name:sub(1, x - 1)
                status = alt .. lvl_name:sub(x + 1)
            end
        end

        gui.box(screen_x, obj_y, screen_x + w, obj_y + h, "clear", color)
        text(screen_x, obj_y - 8, name, color)
        text(screen_x, obj_y + h + 1, status, color)
    end
end

local function display_object(camera_x, obj, obj_type, obj_x, obj_y, obj_state, obj_timer)
    local screen_x = obj_x - camera_x
    local name = obj['name']

    if screen_x >= 0 or screen_x <= 256 then
        local w, h = obj["obj_width"], obj["obj_height"]
        local status = "s" .. to_hex(obj_state)
        local color = COLORS[(obj_type % #COLORS) + 1]

        if name == "book" or name == "wormhole" then
            status = level_id(obj_state)
        end
        if obj_timer ~= nil then
            status = status .. " t" .. obj_timer
        end
        gui.box(screen_x, obj_y, screen_x + w, obj_y + h, "clear", color)
        text(screen_x, obj_y - 8, name, color)
        text(screen_x, obj_y + h + 1, status, color)
    end
end

local function display_camera(x, y)
    local camera_x = memword("camera_x")
    local camera_page = math.floor(camera_x / 256)
    camera_x = camera_x % 256

    local camera_page_min = membyte("camera_page_min")
    local camera_page_max = membyte("camera_page_max")

    text(x, y, "Cam="..camera_page..","..camera_x)
    text(x, y+10, "Lck=" .. camera_page_min .. "," .. camera_page_max)
end

local function display_objects()

    local camera_x = memword("camera_x")
    for i = 1,OBJECT_MAX do
        local bytes = {}
        local offset = i - 1
        for _ = 1,OBJECT_BYTES do
            table.insert(bytes, membyte("obj_x_l", offset))
            offset = offset + OBJECT_MAX
        end

        -- get object type accounting for global objects
        local obj_type = bytes[4]

        local obj = objects[obj_type + 1]
        if obj then
            local obj_x = (bytes[2] * 256) + bytes[1]
            local obj_y = bytes[3]

            local obj_timer = bytes[5]
            local obj_state = bytes[6]
            display_object(camera_x, obj, obj_type, obj_x, obj_y, obj_state, obj_timer)
        end
    end

    for i = 1,TRIGGER_MAX do
        local bytes = {}
        local offset = i - 1
        for _ = 1,TRIGGER_BYTES do
            table.insert(bytes, membyte("trigger_x_l", offset))
            offset = offset + TRIGGER_MAX
        end

        local trigger_type = bytes[3]

        local trigger = triggers[trigger_type + 1]
        if trigger then
            total_triggers = total_triggers + 1
            local trigger_x = (bytes[2] * 256) + bytes[1]
            local trigger_state = bytes[4]
            display_trigger(camera_x, trigger, trigger_type, trigger_x, trigger_state)
        end
    end
end

local function bitflag(value, bit)
    local v = 2 ^ bit
    return AND(value, v) == v
end

local function bitstate(bitstr, value)
    local state = ""
    local function state_val(bit, char)
        if bitflag(value, bit) then
            state = state .. char
        end
    end
    for i=0,8 do
        local char_idx = 9-i
        state_val(i-1, string.sub(bitstr, char_idx, char_idx))
    end
    return state
end

local function display_state(x, y)
    text(x, y, "STE:" .. bitstate("PFBTLGCA", membyte("player_state")))
end

local function display_level_data(x, y)
    local level_zone = membyte("level_zone")
    local level_current = membyte("level_current")
    local level_next = membyte("level_next")
    local level_last = membyte("level_last")

    text(x, y, "LST:" .. level_short_name(level_last))
    text(x, y+9, "LVL:" .. level_short_name(level_current))
    text(x, y+18, "NXT:" .. level_short_name(level_next))
    text(x, y+27, "ZNE:" .. level_zone)
end

local num_active_objects = 0
local active_objects = {}

local function display_object_order(x, y)
    -- only update actives every other frame due to swapping
    if (membyte("frame_counter") % 2) == 1 then
        num_active_objects = membyte("num_active_objects")
        active_objects = {}
        for i = 1,num_active_objects do
            local obj_id = membyte("obj_active", i - 1)
            local obj_type = membyte("obj_type", obj_id)
            table.insert(active_objects, objects[obj_type + 1])
        end
    end
    local OBJ_TYPE_INVALID = 0xFF
    local total_objects = 0
    for obj_id = 1,OBJECT_MAX do
        local obj_type = membyte("obj_type", obj_id-1)
        if obj_type ~= OBJ_TYPE_INVALID then
            total_objects = total_objects + 1
        end
    end
    text(x, y-10, "Tgr="..total_triggers)
    text(x, y, "Objs=" .. num_active_objects .. "/" .. total_objects)
    for i = 1,#active_objects do
        local obj = active_objects[i]
        if obj ~= nil then
            text(x, y+i*9, obj["name"])
        end
    end
end

local function display_player()
    local player_x = membyte("player_x")
    local player_y = membyte("player_y")
    gui.box(
        player_x + 3, player_y + 12,
        player_x + 12, player_y + PLAYER_HEIGHT,
        "clear", "blue")
    -- text(player_x-8, player_y + 38, player_x .. "," .. player_y);
end

local overlays = {
    function() -- display nothing (game only)
    end,
    function() -- bounding boxes
        STATE_PLAY = 0
        if membyte("game_state") == STATE_PLAY then
            display_objects()
            display_player()
        else
            display_player()
        end
    end,
    function() -- puzzle/quest info
        display_camera(4, 12)
        display_object_order(200, 32)
    end,
    function() -- state data
        display_level_data(4, 50)
        display_state(100, 200)
    end,
    function() -- ppu and other junk
        display_ppu(100, 45)
        local bank = membyte("BANK_SAVE")
        local level_bank = membyte("level_bank")
        text(100, 90, "BNK:" .. bank .. " LVL:" .. level_bank)
    end
}

local display_id = 0
local mouse_timer = 0
local display_single = false
local LONG_PRESS = 50

local function frame_update()
    total_triggers = 0
    local state = input.get()
    if state.click == 1 then
        mouse_timer = mouse_timer + 1
    else
        if mouse_timer > 0 and mouse_timer < LONG_PRESS then
            display_id = (display_id + 1) % #overlays
        end
        mouse_timer = 0
    end
    if mouse_timer == LONG_PRESS then
        display_single = not display_single
    end

    if display_single then
        overlays[display_id+1]()
    else
        -- display all overlays up to the current id
        for i = 1,(display_id+1) do
            overlays[i]()
        end
    end
end

read_mem_locations()
randomize_memory()
emu.registerafter(frame_update)

