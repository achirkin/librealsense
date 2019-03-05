package com.intel.realsense.librealsense;

public enum FrameMetadata {
    FRAME_COUNTER(0),
    FRAME_TIMESTAMP(1),
    SENSOR_TIMESTAMP(2),
    ACTUAL_EXPOSURE(3),
    GAIN_LEVEL(4),
    AUTO_EXPOSURE(5),
    WHITE_BALANCE(6),
    TIME_OF_ARRIVAL(7),
    TEMPERATURE(8),
    BACKEND_TIMESTAMP(9),
    ACTUAL_FPS(10),
    FRAME_LASER_POWER(11),
    FRAME_LASER_POWER_MODE(12),
    EXPOSURE_PRIORITY(13),
    EXPOSURE_ROI_LEFT(14),
    EXPOSURE_ROI_RIGHT(15),
    EXPOSURE_ROI_TOP(16),
    EXPOSURE_ROI_BOTTOM(17),
    BRIGHTNESS(18),
    CONTRAST(19),
    SATURATION(20),
    SHARPNESS(21),
    AUTO_WHITE_BALANCE_TEMPERATURE(22),
    BACKLIGHT_COMPENSATION(23),
    HUE(24),
    GAMMA(25),
    MANUAL_WHITE_BALANCE(26),
    POWER_LINE_FREQUENCY(27),
    LOW_LIGHT_COMPENSATION(28);

    private final int mValue;

    private FrameMetadata(int value) { mValue = value; }
    public int value() { return mValue; }
}
