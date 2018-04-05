-- phpMyAdmin SQL Dump
-- version 4.7.7

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET AUTOCOMMIT = 0;
START TRANSACTION;
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Database: `clanmod`
--

-- --------------------------------------------------------

--
-- Table structure for table `jedi_academy`
--

CREATE TABLE `jedi_academy` (
  `user_id` int(255) NOT NULL,
  `kills` int(255) NOT NULL DEFAULT '0',
  `deaths` int(255) NOT NULL DEFAULT '0',
  `duel_wins` int(255) NOT NULL DEFAULT '0',
  `duel_loses` int(255) NOT NULL DEFAULT '0',
  `flag_captures` int(255) NOT NULL DEFAULT '0',
  `ffa_wins` int(255) NOT NULL DEFAULT '0',
  `ffa_loses` int(255) NOT NULL DEFAULT '0',
  `tdm_wins` int(255) NOT NULL DEFAULT '0',
  `tdm_loses` int(255) NOT NULL DEFAULT '0',
  `siege_wins` int(255) NOT NULL DEFAULT '0',
  `siege_loses` int(255) NOT NULL DEFAULT '0',
  `ctf_wins` int(255) NOT NULL DEFAULT '0',
  `ctf_loses` int(255) NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `users`
--

CREATE TABLE `users` (
  `id` int(255) NOT NULL,
  `username` varchar(255) NOT NULL,
  `password` varchar(255) NOT NULL,
  `ipaddress` varchar(20) NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Indexes for dumped tables
--

--
-- Indexes for table `jedi_academy`
--
ALTER TABLE `jedi_academy`
  ADD UNIQUE KEY `user_id` (`user_id`);

--
-- Indexes for table `users`
--
ALTER TABLE `users`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `username` (`username`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `users`
--
ALTER TABLE `users`
  MODIFY `id` int(255) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=11;
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
