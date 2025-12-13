# How to Use Stacking in DWARF II Controller

## ⚠️ Important: GOTO Required!

**Stacking will NOT work without first performing a GOTO!**

Error code `-11513` means: "You must GOTO a target before stacking."

## Why?

Stacking requires the telescope to **track** an object continuously. GOTO:
1. Calibrates the telescope's position
2. Slews to the target object
3. **Starts tracking** - keeps the object centered
4. Enables stacking to work

Without tracking, each frame would show a different position → stacking fails.

## Correct Workflow

### 1. Connect to DWARF II
- Enter IP address (e.g. 192.168.8.30)
- Click "Connect"
- Wait for video stream

### 2. Find a Target (Optional but Recommended: Calibrate First)
**For best results, calibrate before GOTO:**
- Click "Astro" tab
- Click "Start Calibration"
- Wait ~2-3 minutes for calibration
- Status will show "Calibration Complete"

### 3. Search for an Object
- In the Astro tab, use the search box
- Type object name (e.g. "M42", "Andromeda", "Orion")
- Click on the result
- The star map will center on the object

### 4. GOTO the Object
- Click "GOTO" button (or double-click the object on the map)
- Wait for GOTO sequence:
  - Step 1: Calibrating (if not done)
  - Step 2: Slewing (moving to target)
  - Step 3: Centering (fine adjustment)
  - Step 4: Tracking (following the object)
- Status will show "GOTO Complete" when ready

### 5. Configure Stacking Settings
- **Number of Frames**: How many images to stack (e.g. 10, 50, 100)
- **Exposure**: Slide to set exposure time per frame
  - Short exposure (1-5s): Brighter objects, less noise
  - Long exposure (10-30s): Fainter objects, more detail
- **Gain**: Higher gain = more sensitivity, but more noise

### 6. Start Stacking
- Click "Start Stacking"
- You will see:
  - "Preparing camera..." (setting parameters)
  - "Activating Astro mode..." (switching to RAW capture)
  - "Starting stacking..." (beginning capture)
  - Preview stream will turn black (normal!)
  
### 7. Monitor Progress
- Frame counter: "Frame X / Y"
- Stacked frames: Successfully processed
- Rejected frames: Too blurry or misaligned
- Progress bar shows completion

### 8. Stop or Wait for Completion
- Click "Stop Stacking" anytime to abort
- Or wait for all frames to complete
- Preview stream will return

### 9. Download Results
- Click "System" tab → "Open Gallery"
- Find your stacked image
- Click to preview
- Right-click → Download

## Troubleshooting

### Error: "GOTO required! Please use GOTO to a target first"
**Solution:** You skipped step 4. Do GOTO before stacking.

### Error: "Parameters not suitable"
**Solution:** Your exposure/gain settings are invalid. Try:
- Lower exposure (< 30s)
- Gain in range 0-150

### Error: "Dark frame not found"
**Solution:** Some advanced stacking modes need dark frames:
- Go to Astro tab → Dark Frames
- Click "Capture Dark Frame"
- Wait for completion
- Try stacking again

### Stream stays black after stacking
**Solution:** This is normal during stacking (RAW mode). 
- Click "Stop Stacking" to return to preview
- Or wait for completion

### Frames rejected (high rejected count)
**Possible causes:**
- Target is too dim (increase exposure or gain)
- Poor tracking (recalibrate)
- Clouds/fog
- Wind (telescope shaking)

### GOTO fails
**Solutions:**
- Ensure device is level and stable
- Calibrate first (improves accuracy)
- Check target is above horizon
- For planets/moon: Enter correct GPS coordinates

## Tips for Best Results

1. **Calibrate regularly** - especially after moving the telescope
2. **Start with bright targets** - M42, M31, M45 are good for beginners
3. **Use moderate settings** - 10-15s exposure, gain ~80, 20-50 frames
4. **Check weather** - clear, stable nights give best results
5. **Let GOTO finish** - don't rush, wait for "Tracking" status
6. **Monitor rejected frames** - if > 30% rejected, improve conditions

## Recommended First Target: M42 (Orion Nebula)

Easy to find, bright, great for stacking practice:

```
Settings:
- Object: M42 (Orion Nebula)
- Frames: 20
- Exposure: 10s
- Gain: 80
```

1. Search "M42"
2. GOTO → Wait for tracking
3. Set settings above
4. Start Stacking
5. Wait ~4 minutes (20 × 10s + processing)
6. Enjoy your first stacked image!

## Advanced: Unlimited Stacking

Set frames to 0 or very high number:
- Telescope will stack continuously
- Stop manually when satisfied
- Good for faint objects (stack for hours!)

## Error Code Reference

| Code | Meaning | Solution |
|------|---------|----------|
| -11513 | GOTO required | Do GOTO first |
| -11514 | Bad parameters | Adjust exposure/gain |
| -11503 | No dark frame | Capture dark frames |

---

**Have fun stacking! Clear skies! 🌠**
