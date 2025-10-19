## 🧩 MÔ TẢ
Chương trình sử dụng **FreeRTOS** để điều khiển LED nhấp nháy với tần số và độ rộng xung (duty cycle) thay đổi theo thời gian, thông qua cơ chế **Queue**.

- **Task 1:** Thực hiện nhấp nháy LED theo tần số và độ rộng xung nhận được từ Queue.  
- **Task 2:** Định kỳ thay đổi các giá trị tần số và độ rộng xung, sau đó gửi dữ liệu mới qua Queue cho Task 1.  
- Hai task hoạt động song song, kết quả là LED nhấp nháy với tần số và độ sáng (duty) thay đổi sau mỗi 5 giây.

Các giá trị thay đổi luân phiên:
| Lần thay đổi | Tần số (Hz) | Duty (%) |
|:--------------|:------------|:----------|
| 1 | 2 | 10 |
| 2 | 4 | 30 |
| 3 | 6 | 50 |
| 4 | 8 | 70 |
| 5 | 10 | 90 |

---

## 🔌 YÊU CẦU PHẦN CỨNG
- **Vi điều khiển:** STM32F103C8T6 (Blue Pill).  
- **LED:** 1 LED đơn nối vào **chân PB12**, cực âm về GND, cực dương qua điện trở 220Ω.  
- **Nguồn:** 3.3V hoặc 5V tùy board.  
- **Mạch nạp:** ST-Link V2 hoặc tương đương.

**Sơ đồ kết nối:**
| Thành phần | Chân STM32 | Ghi chú |
|:------------|:-----------|:--------|
| LED | PB12 | LED hoạt động mức thấp (ResetBits = Bật, SetBits = Tắt) |
| GND | GND | Nối chung mass hệ thống |

---

## ⚙️ CẤU HÌNH
1. **GPIO:**
   - Cổng **PB12** cấu hình là **Output Push-Pull**, tốc độ 50MHz.
   - LED ban đầu được tắt bằng `GPIO_ResetBits()`.

2. **FreeRTOS:**
   - Tạo **2 Task** và **1 Queue**:
     - `vLEDTask`: nhận dữ liệu từ Queue để điều khiển LED.
     - `vUpdateTask`: gửi tần số và duty mới mỗi 5 giây.
   - Queue chứa 1 phần tử kiểu `Led_config_t`, gồm 2 trường:
     ```c
     typedef struct {
         uint32_t frequency; // tần số LED nháy (Hz)
         uint32_t duty;      // độ rộng xung (%)
     } Led_config_t;
     ```

3. **Hoạt động chính:**
   - Task 1 nhận dữ liệu từ Queue bằng `xQueueReceive()`.
   - Nếu có cấu hình mới, LED thay đổi tần số/duty ngay lập tức.
   - Task 2 gửi dữ liệu bằng `xQueueOverwrite()`, đảm bảo luôn cập nhật giá trị mới nhất.

4. **Thông số hệ thống:**
   - Stack mỗi Task: 128 words.
   - Ưu tiên `Update Task` cao hơn để cập nhật trước.
   - Tần số tick FreeRTOS mặc định: 1 kHz (1 ms/tick).

---

✅ **Kết quả:** LED tại chân PB12 nhấp nháy với tần số và độ sáng thay đổi liên tục theo chu kỳ 5 giây. Chương trình minh họa cách dùng **Queue trong FreeRTOS** để giao tiếp giữa các task.
