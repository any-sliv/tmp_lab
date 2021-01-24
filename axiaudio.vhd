----------------------------------------------------------------------------------
-- Company: 
-- Engineer:
-- 
-- Create Date: 23.01.2021 19:39:31
-- Design Name: 
-- Module Name: axiaudio - Behavioral
-- Project Name: 
-- Target Devices: 
-- Tool Versions: 
-- Description: 
-- 
-- Dependencies: 
-- 
-- Revision:
-- Revision 0.01 - File Created
-- Additional Comments:
-- 
----------------------------------------------------------------------------------


library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity axiaudio is
    port ( 
        xaudio_out : out STD_LOGIC_VECTOR (23 downto 0);
        xaudio_in : in STD_LOGIC_VECTOR (23 downto 0)
        );
end axiaudio;

architecture Behavioral of axiaudio is

begin
    xaudio_out <= xaudio_in;

end Behavioral;
