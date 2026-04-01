
-- to show font with current audio volume percentage, e.g. "75%"

local audio_volume_text = {}

function audio_volume_text.Update(obj, dt)
    local vol = getMasterVolume()                        -- 0.0 to 1.0
    local pct = math.floor(vol * 100 + 0.5)             -- round to nearest int
    setFontText(obj, tostring(pct) .. "%")
end

return audio_volume_text