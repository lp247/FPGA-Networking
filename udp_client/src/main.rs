use std::net::{UdpSocket};
use std::time::Instant;

fn main() -> std::io::Result<()> {
    {
        let socket = UdpSocket::bind("169.254.205.169:35")?;
        let _ = socket.set_read_timeout(None);
        let dst = "169.254.205.2:56928";
        let send_buf = vec![0xAA];
        let mut rec_buf = vec![0; 10];

        let start = Instant::now();

        socket.send_to(&send_buf, &dst)?;
        print!("Sent: ");
        for x in &send_buf {
            print!("{} ", x);
        }
        println!();

        let (read_cnt, _src) = socket.recv_from(&mut rec_buf)?;
        let elapsed_nanos = start.elapsed().as_nanos();
        println!("Received ({}): ", read_cnt);
        for x in &rec_buf {
            print!("{} ", x);
        }
        println!();
        println!("Run time: {} ns", elapsed_nanos);
    }
    Ok(())
}
