package pienv

import "testing"

func TestNormalizeControllerKind(t *testing.T) {
	cases := []struct {
		name string
		raw  string
		want string
	}{
		{"adb", "  AdB  ", ControllerKindAdb},
		{"playcover", "PlayCover", ControllerKindPlayCover},
		{"play_cover alias", "play_cover", ControllerKindPlayCover},
		{"native android", "NATIVE_ANDROID", ControllerKindNativeAndroid},
		{"win32", " Win32 ", ControllerKindWin32},
		{"macos", "MacOS", ControllerKindMacOS},
		{"linux", "LINUX", ControllerKindLinux},
		{"gamepad", "Gamepad", ControllerKindGamepad},
		{"cloudweb", "CloudWeb", ControllerKindCloudWeb},
		{"cloud_web alias", "cloud_web", ControllerKindCloudWeb},
		{"empty", "", ControllerKindUnknown},
		{"unknown", "unknown", ControllerKindUnknown},
	}
	for _, c := range cases {
		t.Run(c.name, func(t *testing.T) {
			if got := NormalizeControllerKind(c.raw); got != c.want {
				t.Fatalf("NormalizeControllerKind(%q) = %q, want %q", c.raw, got, c.want)
			}
		})
	}
}

func TestIsAdbLikeControllerType(t *testing.T) {
	cases := []struct {
		raw  string
		want bool
	}{
		{"Adb", true},
		{"PlayCover", true},
		{"play_cover", true},
		{"native_android", true},
		{"Win32", false},
		{"MacOS", false},
		{"Linux", false},
		{"Gamepad", false},
		{"CloudWeb", false},
		{"cloud_web", false},
		{"", false},
	}
	for _, c := range cases {
		t.Run(c.raw, func(t *testing.T) {
			if got := IsAdbLikeControllerType(c.raw); got != c.want {
				t.Fatalf("IsAdbLikeControllerType(%q) = %v, want %v", c.raw, got, c.want)
			}
		})
	}
}

// controllerKindFromInfo 是 PI_CONTROLLER 缺失时的兜底路径，覆盖 MaaFramework 的 GetInfo 输出。
func TestControllerKindFromInfo(t *testing.T) {
	cases := []struct {
		name string
		info string
		want string
	}{
		{"win32", `{"type":"win32","hwnd":1}`, ControllerKindWin32},
		{"adb", `{"type":"adb","adb_serial":"127.0.0.1:5555"}`, ControllerKindAdb},
		{"native android", `{"type":"native_android","display_id":0}`, ControllerKindNativeAndroid},
		{"playcover", `{"type":"PlayCover"}`, ControllerKindPlayCover},
		{"playcover alias", `{"type":"play_cover"}`, ControllerKindPlayCover},
		{"linux", `{"type":"linux"}`, ControllerKindLinux},
		{"cloudweb runtime", `{"type":"custom","backend":"cloud_web","protocol_version":1}`, ControllerKindCloudWeb},
		{"cloudweb type", `{"type":"cloud_web"}`, ControllerKindCloudWeb},
		{"custom without backend", `{"type":"custom"}`, ControllerKindUnknown},
		// 非法 JSON 时退化到子串探测
		{"non-json native android", `type=native_android`, ControllerKindNativeAndroid},
		{"non-json win32", `type=win32 hwnd=1`, ControllerKindWin32},
		{"non-json cloudweb", `type=custom backend=cloud_web`, ControllerKindCloudWeb},
		// 空串与无法识别的输入都返回 unknown
		{"empty", "", ControllerKindUnknown},
		{"unknown type", `{"type":"dbg"}`, ControllerKindUnknown},
		{"garbage", `not json at all`, ControllerKindUnknown},
	}
	for _, c := range cases {
		t.Run(c.name, func(t *testing.T) {
			if got := controllerKindFromInfo(c.info); got != c.want {
				t.Fatalf("controllerKindFromInfo(%q) = %q, want %q", c.info, got, c.want)
			}
		})
	}
}

func TestCloudWebProfileOf(t *testing.T) {
	controller := &Controller{
		Type: "CloudWeb",
		CloudWeb: &CloudWebConfig{
			Backend:        CloudWebBackendSidecar,
			Profile:        "default",
			UIProfile:      CloudWebUIProfileDefault,
			ServerRegion:   "CN",
			InputSemantics: CloudWebInputSemanticsPC,
		},
	}
	profile, ok := cloudWebProfileOf(controller)
	if !ok {
		t.Fatal("cloudWebProfileOf() did not accept a CloudWeb controller")
	}
	if profile.Transport != CloudWebBackendSidecar || profile.Profile != "default" ||
		profile.UIProfile != CloudWebUIProfileDefault || profile.ServerRegion != "CN" ||
		profile.InputSemantics != CloudWebInputSemanticsPC {
		t.Fatalf("unexpected profile: %+v", profile)
	}

	if _, ok := cloudWebProfileOf(nil); ok {
		t.Fatal("nil controller must not produce a profile")
	}
	if _, ok := cloudWebProfileOf(&Controller{Type: "Win32", CloudWeb: &CloudWebConfig{}}); ok {
		t.Fatal("non-CloudWeb controller must not produce a profile")
	}
	if _, ok := cloudWebProfileOf(&Controller{Type: "CloudWeb"}); ok {
		t.Fatal("CloudWeb without cloudweb object must not produce a profile")
	}
}

func TestParseControllerRuntimeInfo(t *testing.T) {
	info, ok := parseControllerRuntimeInfo(`{
		"backend":"cloud_web",
		"backend_version":"0.1.0",
		"protocol_version":1,
		"session_id":"session-a",
		"profile":"default",
		"ui_profile":"cloud_web.default",
		"capabilities":["frame.latest","input.keyboard"]
	}`)
	if !ok {
		t.Fatal("parseControllerRuntimeInfo() rejected valid info")
	}
	if info.Backend != "cloud_web" || info.BackendVersion != "0.1.0" ||
		info.ProtocolVersion != 1 || info.SessionID != "session-a" ||
		info.Profile != "default" || info.UIProfile != "cloud_web.default" {
		t.Fatalf("unexpected runtime info: %+v", info)
	}
	if !hasCapability(info, CapabilityFrameLatest) || !hasCapability(info, CapabilityInputKeyboard) {
		t.Fatal("expected capabilities were not found")
	}
	if hasCapability(info, "input.relative") {
		t.Fatal("absent capability must return false")
	}
	if _, ok := parseControllerRuntimeInfo(`{"type":"custom"}`); ok {
		t.Fatal("info without backend must be rejected")
	}
	if _, ok := parseControllerRuntimeInfo("not json"); ok {
		t.Fatal("invalid JSON must be rejected")
	}
}
